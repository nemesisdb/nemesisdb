#ifndef NDB_CORE_KVSERVER_H
#define NDB_CORE_KVSERVER_H

#include <functional>
#include <vector>
#include <set>
#include <tuple>
#include <latch>
#include <thread>
#include <chrono>
#include <iostream>
#include <uwebsockets/App.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvHandler.h>
#include <core/sh/ShHandler.h>
#include <core/kv/SvCommands.h>
#include <core/Persistance.h>
#include <core/NemesisConfig.h>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core { namespace kv {

namespace sv = nemesis::core::sv;


template<bool HaveSessions>
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


  constexpr bool hasSessions () const
  {
    if constexpr (HaveSessions) 
      return true;
    else
      return false;
  }


  void stop()
  {
    m_run = false;

    try
    {
      if (m_monitorTimer)
        us_timer_close(m_monitorTimer);

      for (auto ws : m_wsClients)
        ws->end(1000); // calls wsApp.close()

      for(auto sock : m_sockets)
        us_listen_socket_close(0, sock);
    }
    catch (const std::exception& ex)
    {
      //ignore, shutting down
    }
    

    m_wsClients.clear();
    m_sockets.clear();
    m_monitorTimer = nullptr;
  }


  bool run (NemesisConfig& config, std::shared_ptr<Sessions> sessions)
  {
    m_sessions = sessions;
    m_config = config.cfg;

    if (!init(config.cfg))
      return false;

    if (config.load())
    {
      if (auto [ok, msg] = load(config); !ok)
      {
        PLOGF << msg;
        return false;
      }
    }

    const auto [ip, port, maxPayload] = NemesisConfig::wsSettings(config.cfg);
    const std::size_t preferredCore = NemesisConfig::preferredCore(config.cfg); 
    std::size_t core = 0;
    
    if (const auto maxCores = std::thread::hardware_concurrency(); maxCores == 0)
      PLOGE << "Could not acquire available cores";
    else if (preferredCore > maxCores)
      PLOGE << "'core' value in config is above maximum available: " << maxCores;
    else
      core = preferredCore;


    if (!startWsServer(ip, port, maxPayload, core))
      return false;
        

    #ifndef NDB_UNIT_TEST
    PLOGI << "Ready";
    #endif

    return true;
  }

  
  private:

    bool init(const njson& config)
    {
      bool init = true;

      try
      {
        if (NemesisConfig::persistEnabled(config))
        {
          // test we can write to the persist path
          if (std::filesystem::path path {NemesisConfig::savePath(config)}; !std::filesystem::exists(path) || !std::filesystem::is_directory(path))
          {
            PLOGF << "save path is not a directory or does not exist";
            return false;
          }
          else
          {
            const auto filename = std::to_string(NemesisClock::now().time_since_epoch().count());
            const fs::path fullPath{path / filename};

            if (std::ofstream testStream{fullPath}; !testStream.good())
            {
              PLOGF << "Failed test write to save path: " << path;
              return false;
            }
            else
              std::filesystem::remove(fullPath);
          }
        }
        
        m_kvHandler = std::make_shared<kv::KvHandler<HaveSessions>> (config, m_sessions);
        m_shHandler = std::make_shared<ShHandler>(config, m_sessions);
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


      auto listen = [this, ip, port, &listening, &startLatch, maxPayload]()
      {
        auto wsApp = uWS::App().ws<WsSession>("/*",
        {
          .compression = uWS::DISABLED, // consider uWS::SHARED_COMPRESSOR
          .maxPayloadLength = maxPayload,
          .idleTimeout = 180, // TODO should be configurable?
          .maxBackpressure = 24 * 1024 * 1024,
          // handlers
          .upgrade = nullptr,
          .open = [this](KvWebSocket * ws)
          {
            m_wsClients.insert(ws);
          },          
          .message = [this](KvWebSocket * ws, std::string_view message, uWS::OpCode opCode)
          {
            onMessage(ws, message, opCode);
          },
          .drain = [](KvWebSocket * ws)
          {
            // TODO
            // Doing so properly requires changes because must check return value of ws->send():
            //  - if it returns BACKPRESSURE we must (I think):
            //    - queue the message
            //    - when drain() is called we can begin sending from queue
            //
            // 1. send() in KvCommon is useless
            // 2. Perhaps move send() to here
            // 3. All functions return the response (in onMessage()), 
            // 4. onMessage() calls send(), which will handle queueing etc
            //
            // or, create a struct Response which encapsulates the json rsp but also a flag for error condition, which 
            // may be useful
          },
          .close = [this](KvWebSocket * ws, int /*code*/, std::string_view /*message*/)
          {
            ws->getUserData()->connected->store(false);

            // when we shutdown, we have to call ws->end() to close each client otherwise uWS loop doesn't return,
            // but when we call ws->end(), this lambda is called, so we need to avoid mutex deadlock with this flag
            if (m_run)
              m_wsClients.erase(ws);
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

          if constexpr (HaveSessions)
          {
            #ifndef NDB_UNIT_TEST

            const auto periodMs = chrono::milliseconds{5'000};

            PLOGD << "Creating monitor timer for " << periodMs.count() << "ms";

            if (m_monitorTimer = us_create_timer((struct us_loop_t *) uWS::Loop::get(), 0, sizeof(TimerData)); m_monitorTimer)
            {
              auto * timerData = reinterpret_cast<TimerData *> (us_timer_ext(m_monitorTimer));
              timerData->kvHandler = m_kvHandler;

              us_timer_set(m_monitorTimer, [](struct us_timer_t * timer)
              {
                if (auto * timerData = reinterpret_cast<TimerData *> (us_timer_ext(timer)); timerData)
                  timerData->kvHandler->monitor();
              },
              periodMs.count(),
              periodMs.count());
            }
            else
              timerSet = false;

            #endif
          }

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
        ws->send(createErrorResponse(RequestStatus::OpCodeInvalid).to_string(), kv::WsSendOpCode);
      else
      {
        jsoncons::json_decoder<njson> decoder;
        jsoncons::json_string_reader reader(message, decoder);
        
        try
        {
          if (reader.read(); !decoder.is_valid())
            ws->send(createErrorResponse(RequestStatus::JsonInvalid).to_string(), kv::WsSendOpCode);
          else
          {
            handleMessage(ws, decoder.get_result());
          }
        }
        catch (const jsoncons::ser_error& jsonEx)
        {
          ws->send(createErrorResponse(RequestStatus::JsonInvalid).to_string(), kv::WsSendOpCode);
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
            const auto type = command.substr(0, pos);

            if (type == "SH")
            {
              const Response response = m_shHandler->handle(ws, command, request);
              send(ws, response.rsp);
            }
            else if (type == "KV")
            {
              const Response response = m_kvHandler->handle(ws, command, request);
              send(ws, response.rsp);
            }
            else if (command == sv::InfoReq)
            {
              // vomit, but this is how jsoncons initialises json objects
              // the json_object_arg is a tag, followed by initializer_list<pair<string, njson>>
              // Can create Prepared in one go, but split for [relative] readability. Copy children from Info into Prepared
              static const njson Info {jsoncons::json_object_arg, {
                                                                    {"st", toUnderlying(RequestStatus::Ok)},  // for compatibility with APIs and consistancy
                                                                    {"serverVersion",   NEMESIS_VERSION},
                                                                    {"sessionsEnabled", NemesisConfig::sessionsEnabled(m_config)},
                                                                    {"persistEnabled",  NemesisConfig::persistEnabled(m_config)}
                                                                  }};

              static const njson Prepared {jsoncons::json_object_arg, {{sv::InfoRsp, {jsoncons::json_object_arg,  Info.object_range().cbegin(),
                                                                                                                  Info.object_range().cend()}}}}; 
              
              send(ws, Prepared);
            }
          }
        }
      }
    }


    std::tuple<bool, const std::string_view> load(const NemesisConfig& config)
    {
      if (const auto [valid, msg] = validatePreLoad(config.loadName, config.loadPath, HaveSessions); !valid)
      {
        return {false, msg};
      }
      else
      {
        const auto [root, md, data, pathsValid] = getLoadPaths(config.loadPath / config.loadName);
        const auto loadResult = m_kvHandler->internalLoad(config.loadName, data);
        const auto success = loadResult.status == RequestStatus::LoadComplete;

        PLOGI << "-- Load --";
    
        if (success)
        {
          PLOGI << "Status: Success";
          if constexpr (HaveSessions)
          {
            PLOGI << "Sessions: " << loadResult.nSessions ;
          }
            
          PLOGI << "Keys: " << loadResult.nKeys ;
          PLOGI << "Duration: " << chrono::duration_cast<std::chrono::milliseconds>(loadResult.duration).count() << "ms";
        }
        else
          PLOGI << "Status: Fail";

        PLOGI << "----------";
        
        return {success, ""};
      }     
    }
    

  private:
    struct TimerData
    {
      std::shared_ptr<KvHandler<HaveSessions>> kvHandler;
    };

    std::unique_ptr<std::jthread> m_thread;
    std::vector<us_listen_socket_t *> m_sockets;
    std::set<KvWebSocket *> m_wsClients;    
    std::atomic_bool m_run;
    us_timer_t * m_monitorTimer{};
    // TODO profile runtime cost of shared_ptr vs raw ptr
    std::shared_ptr<KvHandler<HaveSessions>> m_kvHandler;
    std::shared_ptr<ShHandler> m_shHandler;
    std::shared_ptr<Sessions> m_sessions;
    njson m_config;
};


using KvServer = Server<false>;
using KvSessionServer = Server<true>;


}
}
}

#endif
