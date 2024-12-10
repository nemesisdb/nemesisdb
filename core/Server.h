#ifndef NDB_CORE_KVSERVER_H
#define NDB_CORE_KVSERVER_H

#include <functional>
#include <vector>
#include <tuple>
#include <latch>
#include <thread>
#include <iostream>
#include <uwebsockets/App.h>
#include <core/Persistance.h>
#include <core/NemesisConfig.h>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvHandler.h>
#include <core/sh/ShHandler.h>
#include <core/sh/ShSessions.h>
#include <core/sv/SvCommands.h>
#include <core/arr/ArrHandler.h>
#include <core/arr/ArrCommands.h>



namespace nemesis { 


namespace kvCmds = nemesis::kv::cmds;
namespace shCmds = nemesis::sh;
namespace svCmds = nemesis::sv;
namespace arrCmds = nemesis::arr::cmds;



class Server
{

public:
  Server() : m_run(true)
  {
    
  }


  ~Server()
  {
    stop();
  }



  void stop()
  {
    m_run = false;

    try
    {
      if (m_monitorTimer)
        us_timer_close(m_monitorTimer);

      for (auto ws : m_clients)
        ws->end(1000); // calls wsApp.close()

      for(auto sock : m_sockets)
        us_listen_socket_close(0, sock);
    }
    catch (const std::exception& ex)
    {
      //ignore, shutting down
    }
    

    m_clients.clear();
    m_sockets.clear();
    m_monitorTimer = nullptr;
  }


  bool run (const Settings& settings, std::shared_ptr<sh::Sessions> sessions)
  {
    m_sessions = sessions;
    m_settings = settings;


    if (!init())
      return false;

    if (m_settings.loadOnStartup)
    {
      if (auto [ok, msg] = startupLoad(); !ok)
      {
        PLOGF << msg;
        return false;
      }
    }

    const auto [ip, port, maxPayload] = m_settings.interface;
    std::size_t core = 0;
    
    if (const auto maxCores = std::thread::hardware_concurrency(); maxCores == 0)
      PLOGE << "Could not acquire available cores";
    else if (m_settings.preferredCore > maxCores)
      PLOGE << "'core' value in config is above maximum available: " << maxCores;
    else
      core = m_settings.preferredCore;


    if (!startWsServer(ip, port, maxPayload, core))
      return false;
        

    #ifndef NDB_UNIT_TEST
    PLOGI << "Ready";
    #endif

    return true;
  }

  
  private:

    bool init()
    {
      bool init = true;

      try
      {
        if (m_settings.persistEnabled)
        {
          // test we can write to the persist path
          if (std::filesystem::path path {m_settings.persistPath}; !std::filesystem::exists(path) || !std::filesystem::is_directory(path))
          {
            PLOGF << "persist path is not a directory or does not exist";
            return false;
          }
          else
          {
            const auto filename = std::to_string(NemesisClock::now().time_since_epoch().count());
            const fs::path fullPath{path / filename};

            if (std::ofstream testStream{fullPath}; !testStream.good())
            {
              PLOGF << "Failed test write to persist path: " << path;
              return false;
            }
            else
              std::filesystem::remove(fullPath);
          }
        }
        
        m_kvHandler = std::make_shared<kv::KvHandler>(m_settings);
        m_shHandler = std::make_shared<sh::ShHandler>(m_settings, m_sessions);
        m_objectArrHandler = std::make_shared<arr::OArrHandler>(m_settings);
        m_intArrHandler = std::make_shared<arr::IntArrHandler>(m_settings);
        m_strArrHandler = std::make_shared<arr::StrArrHandler>(m_settings);
        m_sortedIntArrHandler = std::make_shared<arr::SortedIntArrHandler>(m_settings);
      }
      catch(const std::exception& e)
      {
        PLOGF << e.what();
        init = false;
      }

      return init;
    }
    
    
    bool startWsServer (const std::string& ip, const int port, const unsigned int maxPayload, const std::size_t core)
    {
      bool listening{false};
      std::latch startLatch (1);

      // TODO consider using path per command type, i.e. all kv commands go to /kv and similar for /sh and /sv
      //      will this reduce checks, if not then don't bother (command has to be checked anyway)
      auto listen = [this, ip, port, &listening, &startLatch, maxPayload]()
      {
        auto wsApp = uWS::App().ws<WsSession>("/*",
        {
          // settings
          .compression = uWS::DISABLED, // TODO consider uWS::SHARED_COMPRESSOR
          .maxPayloadLength = maxPayload,
          .idleTimeout = 180, // TODO should be configurable?
          .maxBackpressure = 128 * 1024,  // TODO reduce when draining handles properly
          // handlers
          .open = [this](KvWebSocket * ws)
          {
            m_clients.insert(ws);
          },          
          .message = [this](KvWebSocket * ws, std::string_view message, uWS::OpCode opCode)
          {
            onMessage(ws, message, opCode);
          },
          .drain = [](KvWebSocket * ws)
          {
            // TODO
            // https://github.com/uNetworking/uWebSockets/blob/master/misc/READMORE.md
            //
            // Something like:
            //
            //  a) ws->getBufferedAmount() is the bytes that have not been sent (i.e. are buffered)
            //  b) keep sending until ws->getBufferedAmount() >= maxBackpressure (perhaps check current message can be sent within the backpressure)
            //  c) either drop or queue messages
          },
          .close = [this](KvWebSocket * ws, int /*code*/, std::string_view /*message*/)
          {
            ws->getUserData()->connected->store(false);

            // when we shutdown, we have to call ws->end() to close each client otherwise uWS loop doesn't return,
            // but when we call ws->end(), this lambda is called, so we need to avoid mutex deadlock with this flag
            if (m_run)
              m_clients.erase(ws);
          }
        })
        .listen(ip, port, [this, port, &listening, &startLatch](auto * listenSocket)
        {
          if (listenSocket)
          {
            m_sockets.push_back(listenSocket);

            us_socket_t * socket = reinterpret_cast<us_socket_t *>(listenSocket); // this cast is safe
            listening = us_socket_is_closed(0, socket) == 0U;
          }

          startLatch.count_down();
        });
        

        if (!wsApp.constructorFailed())
        {
          bool timerSet = true;
          
          #ifndef NDB_UNIT_TEST
            const auto periodMs = chrono::milliseconds{5'000};

            PLOGD << "Creating monitor timer for " << periodMs.count() << "ms";

            if (m_monitorTimer = us_create_timer((struct us_loop_t *) uWS::Loop::get(), 0, sizeof(TimerData)); m_monitorTimer)
            {
              auto * timerData = reinterpret_cast<TimerData *> (us_timer_ext(m_monitorTimer));
              timerData->shHandler = m_shHandler;

              us_timer_set(m_monitorTimer, [](struct us_timer_t * timer)
              {
                if (auto * timerData = reinterpret_cast<TimerData *> (us_timer_ext(timer)); timerData)
                  timerData->shHandler->monitor();
              },
              periodMs.count(),
              periodMs.count());
            }
            else
              timerSet = false;
          #endif
          

          if (timerSet)
            wsApp.run();
        }
      };


      bool started = false;

      try
      {
        m_thread.reset(new std::jthread(listen));

        startLatch.wait();

        if (!listening)
        {
          PLOGE << "Failed to start WS server";
        }          
        else if (!setThreadAffinity(m_thread->native_handle(), core))
        {
          PLOGE << "Failed to assign server to core " << core;
        }
        else
        {
          PLOGI << "CPU Core: " << core;
          started = true;
        }
      }
      catch(const std::exception& e)
      {
        PLOGF << "Failed to start WS thread:\n" << e.what();
      }

      return started;
    }


    void onMessage(KvWebSocket * ws, std::string_view message, uWS::OpCode opCode)
    { 
      if (opCode != uWS::OpCode::TEXT)
        send(ws, createErrorResponse(RequestStatus::OpCodeInvalid));
      else
      {
        jsoncons::json_decoder<njson> decoder;
        jsoncons::json_string_reader reader(message, decoder);
        
        try
        {
          if (reader.read(); !decoder.is_valid())
            send(ws, createErrorResponse(RequestStatus::JsonInvalid));
          else
            handleMessage(ws, decoder.get_result());
        }
        catch (const jsoncons::ser_error& jsonEx)
        {
          send(ws, createErrorResponse(RequestStatus::JsonInvalid));
        }
        catch (const std::exception& ex)
        {
          send(ws, createErrorResponse(RequestStatus::Unknown));
        }          
      }
    }

    
    void handleMessage(KvWebSocket * ws, njson request)
    {
      // top level must be an object with one child
      if (!request.is_object() || request.size() != 1U)
        send(ws, createErrorResponse(RequestStatus::CommandSyntax));
      else
      {
        const std::string& command = request.object_range().cbegin()->key();

        if (!request.at(command).is_object())
          send(ws, createErrorResponse(command+"_RSP", RequestStatus::CommandSyntax));
        else
        {
          if (const auto pos = command.find('_'); pos == std::string::npos)
            send(ws, createErrorResponse(command+"_RSP", RequestStatus::CommandSyntax));
          else
          {
            //const auto type = command.substr(0, pos);
            const auto type = std::string_view(command.substr(0, pos));
            
            if (type == kvCmds::KvIdent)
            {
              const Response response = m_kvHandler->handle(command, request);
              send(ws, response.rsp);
            }
            else if (type == arrCmds::OArrayIdent)
            {
              const Response response = m_objectArrHandler->handle(command, request);
              send(ws, response.rsp);
            }
            else if (type == arrCmds::IntArrayIdent)
            {
              const Response response = m_intArrHandler->handle(command, request);
              send(ws, response.rsp);
            }
            else if (type == arrCmds::StrArrayIdent)
            {
              const Response response = m_strArrHandler->handle(command, request);
              send(ws, response.rsp);
            }
            else if (type == arrCmds::SortedIntArrayIdent)
            {
              const Response response = m_sortedIntArrHandler->handle(command, request);
              send(ws, response.rsp);
            }
            else if (type == shCmds::ShIdent)
            {
              const Response response = m_shHandler->handle(command, request);
              send(ws, response.rsp);
            }
            else if (command == sv::cmds::InfoReq)  // only one SV command at the moment
            {
              // the json_object_arg is a tag, followed by initializer_list<pair<string, njson>>
              static const njson Info = {jsoncons::json_object_arg, {
                                                                      {"st", toUnderlying(RequestStatus::Ok)},  // for compatibility with APIs and consistency
                                                                      {"serverVersion",   NEMESIS_VERSION},
                                                                      {"persistEnabled",  m_settings.persistEnabled}
                                                                    }};
              static const njson Prepared {jsoncons::json_object_arg, {{sv::cmds::InfoRsp, Info}}}; 

              send(ws, Prepared);
            }
            else  [[unlikely]]
            {
              static const njson Prepared {createErrorResponse(command+"_RSP", RequestStatus::CommandNotExist)};
              send(ws, Prepared);
            }
          }
        }
      }
    }


    std::tuple<bool, const std::string_view> startupLoad()
    {
      PLOGI << "-- Load --";

      const auto& loadName = m_settings.startupLoadName;
      const auto& loadPath = m_settings.startupLoadPath;

      if (PreLoadInfo info = validatePreLoad(loadName, loadPath); !info.valid)
      {
        return {false, info.err};
      }
      else
      {
        LoadResult loadResult;

        if (info.dataType == SaveDataType::SessionKv)
          loadResult = m_shHandler->internalLoad(loadName, info.paths.data);
        else
          loadResult = m_kvHandler->internalLoad(loadName, info.paths.data);
        
        const auto success = loadResult.status == RequestStatus::LoadComplete;
    
        if (!success)
          PLOGI << "Status: Fail";
        else
        {
          PLOGI << "Status: Success";

          if (info.dataType == SaveDataType::SessionKv)
          {
            PLOGI << "Sessions: " << loadResult.nSessions ;
          }              
            
          PLOGI << "Keys: " << loadResult.nKeys ;
          PLOGI << "Duration: " << chrono::duration_cast<std::chrono::milliseconds>(loadResult.duration).count() << "ms";
        }

        PLOGI << "----------";
        
        return {success, ""};
      }    
    }
    

    ndb_always_inline void send (KvWebSocket * ws, const njson& msg)
    {
      ws->send(msg.to_string(), WsSendOpCode);
    }


  private:
    struct TimerData
    {
      std::shared_ptr<sh::ShHandler> shHandler;
    };

    std::unique_ptr<std::jthread> m_thread;
    std::vector<us_listen_socket_t *> m_sockets;
    std::set<KvWebSocket *> m_clients;    
    std::atomic_bool m_run;
    us_timer_t * m_monitorTimer{};
    // TODO profile runtime cost of shared_ptr vs raw ptr
    std::shared_ptr<kv::KvHandler> m_kvHandler;
    std::shared_ptr<sh::ShHandler> m_shHandler;
    std::shared_ptr<arr::OArrHandler> m_objectArrHandler;
    std::shared_ptr<arr::IntArrHandler> m_intArrHandler;
    std::shared_ptr<arr::StrArrHandler> m_strArrHandler;
    std::shared_ptr<arr::SortedIntArrHandler> m_sortedIntArrHandler;
    std::shared_ptr<sh::Sessions> m_sessions;
    Settings m_settings;
};

}


#endif
