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
#include <core/NemesisConfig.h>


namespace nemesis { namespace core { namespace kv {


template<bool HaveSessions>
class Server
{
  struct Handler
  {
    KvHandler<HaveSessions> * handler {nullptr};
  } ;

  static inline Handler s_kvHandler;  // used by the callback handler for us_timer_set(), a C library


public:
  Server() : m_run(true)
  {
    kv::serverStats = nullptr;
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

      for (auto ws : m_wsClients)
        ws->end(1000); // calls wsApp.close()

      for(auto sock : m_sockets)
        us_listen_socket_close(0, sock);

      if (s_kvHandler.handler)
        delete s_kvHandler.handler;

      if (kv::serverStats)
        delete kv::serverStats;
    }
    catch (...)
    {
      // ignore, shutting down
    }
    

    m_wsClients.clear();
    m_sockets.clear();
    m_monitorTimer = nullptr;
    s_kvHandler.handler = nullptr;
    kv::serverStats = nullptr;
  }


  bool run (NemesisConfig& config)
  {
    if (!init(config.cfg))
      return false;

    if constexpr(HaveSessions) 
    {
      if ((config.load()))
      {
        if (auto [ok, msg] = load(config); !ok)
        {
          PLOGF << msg;
          return false;
        }
      }
    }
    

    const unsigned int maxPayload = config.cfg["kv"]["maxPayload"].as<unsigned int>();
    const std::string ip = config.cfg["kv"]["ip"].as_string();
    const int port = config.cfg["kv"]["port"].as<unsigned int>();


    if (!startWsServer(ip, port, maxPayload, serverStats))
      return false;
        

    #ifndef NDB_UNIT_TEST
    PLOGI << "Ready";
    #endif

    return true;
  }

  
  private:

    // used by uWS timer callback, a C function
    static void onMonitor (struct us_timer_t *)
    {
      if constexpr (HaveSessions)
        s_kvHandler.handler->monitor();
    }


    bool init(const njson& config)
    {
      if constexpr (HaveSessions)
      {
        if (NemesisConfig::kvSaveEnabled(config))
        {
          // test we can write to the kv save path
          if (std::filesystem::path path {NemesisConfig::kvSavePath(config)}; !std::filesystem::exists(path) || !std::filesystem::is_directory(path))
          {
            PLOGF << "kv:session::save::path is not a directory or does not exist";
            return false;
          }
          else
          {
            const auto filename = createUuid();
            std::filesystem::path fullPath{path};
            fullPath /= filename;

            if (std::ofstream testStream{fullPath}; !testStream.good())
            {
              PLOGF << "Cannot write to kv:session::save::path";
              return false;
            }
            else
              std::filesystem::remove(fullPath);
          }
        }
      }
      

      kv::serverStats = new kv::ServerStats;
      s_kvHandler.handler = new kv::KvHandler<HaveSessions> {config}; // TODO

      return true;
    }
    
    
    bool startWsServer (const std::string& ip, const int port, const unsigned int maxPayload, kv::ServerStats * stats)
    {
      const std::size_t core = 0;

      bool listening{false};
      std::latch startLatch (1);

      auto listen = [this, ip, port, &listening, &startLatch, maxPayload, stats]()
      {
        char requestBuffer[NEMESIS_KV_MAXPAYLOAD] = {};
        
        auto wsApp = uWS::App().ws<WsSession>("/*",
        {
          .compression = uWS::DISABLED,
          .maxPayloadLength = maxPayload,
          .idleTimeout = 180, // TODO should be configurable?
          .maxBackpressure = 24 * 1024 * 1024,
          // handlers
          .upgrade = nullptr,
          .open = [this](KvWebSocket * ws)
          {
            m_wsClients.insert(ws);
          },          
          .message = [this, stats, &requestBuffer](KvWebSocket * ws, std::string_view message, uWS::OpCode opCode)
          { 
            ++serverStats->queryCount;

            if (opCode != uWS::OpCode::TEXT)
              ws->send(createErrorResponse(RequestStatus::OpCodeInvalid).to_string(), kv::WsSendOpCode);
            else
            {
              jsoncons::json_decoder<njson> decoder;
              jsoncons::json_string_reader reader(message, decoder);
              
              if (reader.read(); !decoder.is_valid())
                ws->send(createErrorResponse(RequestStatus::JsonInvalid).to_string(), kv::WsSendOpCode);
              else
              {
                njson request = decoder.get_result();

                if (request.empty() || !request.is_object()) //i.e. top level not an object
                  ws->send(createErrorResponse(RequestStatus::CommandSyntax).to_string(), kv::WsSendOpCode);
                else if (request.size() > 1U)
                  ws->send(createErrorResponse(RequestStatus::CommandMultiple).to_string(), kv::WsSendOpCode);
                else
                {
                  const auto& commandName = request.object_range().cbegin()->key();

                  if (!request.at(commandName).is_object())
                    ws->send(createErrorResponse(commandName+"_RSP", RequestStatus::CommandType).to_string(), kv::WsSendOpCode);
                  else if (const auto status = s_kvHandler.handler->handle(ws, commandName, request); status != RequestStatus::Ok)
                    ws->send(createErrorResponse(commandName+"_RSP", status).to_string(), kv::WsSendOpCode);
                }
              }
            }
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
          if constexpr (HaveSessions)
          {
            #ifndef NDB_UNIT_TEST

            const auto periodMs = chrono::duration_cast<chrono::milliseconds>(chrono::seconds {5});

            PLOGD << "Creating monitor timer for " << periodMs.count() << "ms";

            m_monitorTimer = us_create_timer((struct us_loop_t *) uWS::Loop::get(), 0, 0);

            us_timer_set(m_monitorTimer, Server<HaveSessions>::onMonitor, periodMs.count(), periodMs.count());

            #endif
          }
                  
          wsApp.run();
        }
          
      };


      bool started = false;
      try
      {
        m_thread.reset(new std::jthread(listen));

        startLatch.wait();

        if (listening)
        {
          if (!setThreadAffinity(m_thread->native_handle(), core))
            PLOGW << "Failed to assign io thread to core " << core;
          else
            started = true;
        }
        else
          PLOGE << "Failed to start WS server";
      }
      catch(const std::exception& e)
      {
        PLOGF << "Failed to start WS thread:\n" << e.what();
      }

      return started;
    }


    std::tuple<bool, std::string> load(const NemesisConfig& config)
      requires(HaveSessions)
    {
      const fs::path root = config.loadPath / config.loadName;

      PLOGI << "Loading " << root;

      if (!fs::exists(root))
        return {false, "Load name does not exist"};
      
      if (!fs::is_directory(root))
        return {false, "Path to load name is not a directory"};

      
      // loadRoot may contain several saves (i.e. SH_SAVE used multiple times with the same 'name'),
      // so use getDefaultDataSetPath() to get the most recent
      const auto datasetRoot = getDefaultDataSetPath(root);

      // check data dir and metadata file
      const fs::path data = datasetRoot / "data";
      const fs::path mdFile = datasetRoot / "md" / "md.json";

      if (!(fs::exists(data) && fs::is_directory(data)))
        return {false, "'data' directory does not exist or is not a directory"};
      
      if (!fs::exists(mdFile))
        return {false, "metadata does not exist"};


      PLOGI << "Reading metadata in " << mdFile;

      std::ifstream mdStream {mdFile};
      const auto mdJson = njson::parse(mdStream);

      if (!mdJson.contains("status") || !mdJson["status"].is_uint64())
        return {false, "Metadata file invalid"};
      else if (mdJson["status"] != toUnderlying(KvSaveStatus::Complete))
        return {false, "Cannot load: save was incomplete"};
      else
      {
        const auto loadResult = s_kvHandler.handler->internalLoad(config.loadName, data);
        const auto success = loadResult.status == RequestStatus::LoadComplete;

        PLOGI << "-- Load --";
    
        if (success)
        {
          PLOGI << "Status: Success";
          PLOGI << "Sessions: " << loadResult.nSessions ;
          PLOGI << "Keys: " << loadResult.nKeys ;
          PLOGI << "Duration: " << chrono::duration_cast<std::chrono::milliseconds>(loadResult.loadTime).count() << "ms";
        }
        else
          PLOGI << "Status: Fail";

        PLOGI << "----------";
        
        return {success, ""};
      }
    }
    

  private:
    std::unique_ptr<std::jthread> m_thread;
    std::vector<us_listen_socket_t *> m_sockets;
    std::set<KvWebSocket *> m_wsClients;    
    std::atomic_bool m_run;
    us_timer_t * m_monitorTimer{nullptr};
};


using KvServer = Server<false>;
using KvSessionServer = Server<true>;


}
}
}

#endif
