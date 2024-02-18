#ifndef NDB_CORE_TSSERVER_H
#define NDB_CORE_TSSERVER_H

#include <core/NemesisCommon.h>
#include <core/ts/TsCommon.h>
#include <core/ts/TsHandler.h>


namespace nemesis { namespace core { namespace ts {


class TsServer 
{
public:
  
  TsServer () : m_run(false)
  {

  }


  ~TsServer()
  {
    try
    {
      stop();  
    }
    catch(const std::exception& e)
    {
      PLOGF << "Error during shutdown";
    }
  }


  void stop()
  {
    m_run = false;

    // if (m_monitor.joinable())
    //   m_monitor.join();

    {
      std::scoped_lock lck{m_wsClientsMux};
      for (auto ws : m_wsClients)
        ws->end(1000); // calls AsyncSocket::shutdown()

      m_wsClients.clear();
    }

    {
      std::scoped_lock lck{m_socketsMux};

      for(auto sock : m_sockets)
        us_listen_socket_close(0, sock);

      m_sockets.clear();
    }

    for(auto& thread : m_threads)
      thread->join();
  }


  bool run (const NemesisConfig& config)
  {
    m_run = true;
    m_serverStats = std::make_shared<ServerStats>();
    m_handler = std::make_shared<TsHandler>(config.cfg);

    const unsigned int maxPayload = config.cfg["kv"]["maxPayload"].as<unsigned int>();
    const std::string ip = config.cfg["kv"]["ip"].as_string();
    const int port = config.cfg["kv"]["port"].as<unsigned int>();


    std::size_t nIoThreads = 1;
    std::size_t listenSuccess{0U};
    std::atomic_ref listenSuccessRef{listenSuccess};
    std::latch startLatch (nIoThreads);


    for (std::size_t i = 0U, core = 0U ; i < nIoThreads ; ++i, ++core)
    {
      auto * thread = new std::jthread([this, ip, port, &listenSuccessRef, &startLatch, maxPayload, serverStats = m_serverStats, handler = m_handler]()
      {
        auto wsApp = uWS::App().ws<WsSession>("/*",
        {
          .compression = uWS::DISABLED,
          .maxPayloadLength = maxPayload,
          .idleTimeout = 180,
          .maxBackpressure = 24 * 1024 * 1024,
          // handlers
          .upgrade = nullptr,
          .open = [this](TsWebSocket * ws)
          {
            std::scoped_lock lck{m_wsClientsMux};
            m_wsClients.insert(ws);
          },          
          .message = [this, serverStats, handler](TsWebSocket * ws, std::string_view message, uWS::OpCode opCode)
          {   
            ++serverStats->queryCount;

            if (opCode != uWS::OpCode::TEXT)
              ws->send(createErrorResponse(RequestStatus::OpCodeInvalid).to_string(), WsSendOpCode);
            else
            {
              njson request = njson::null();

              try
              {
                request = njson::parse(message);
              }
              catch (...)
              {
              }
              
              if (request.is_null())
                PLOGD << "Duff json";
              else if (request.empty() || !request.is_object())
                PLOGD << "Query empty or not an object";
              else
              {
                const auto& commandName = request.object_range().cbegin()->key();

                PLOGD << commandName;

                if (request.at(commandName).empty())
                  PLOGD << "Command empty";
                else if (const auto status = handler->handle(ws, commandName, std::move(request)); status != TsRequestStatus::Ok)
                  PLOGD << "Some handle error";
              }
            }
          },
          .close = [this](TsWebSocket * ws, int /*code*/, std::string_view /*message*/)
          {
            ws->getUserData()->connected->store(false);

            // when we shutdown, we have to call ws->end() to close each client otherwise uWS loop doesn't return,
            // but when we call ws->end(), this lambda is called, so we need to avoid mutex deadlock with this flag
            if (m_run)
            {
              std::scoped_lock lck{m_wsClientsMux};
              m_wsClients.erase(ws);
            }
          }
        })
        .listen(ip, port, [this, port, &listenSuccessRef, &startLatch](auto * listenSocket)
        {
          if (listenSocket)
          {
            {
              std::scoped_lock lck{m_socketsMux};
              m_sockets.push_back(listenSocket);
            }

            us_socket_t * socket = reinterpret_cast<us_socket_t *>(listenSocket); // this cast is safe
            listenSuccessRef += us_socket_is_closed(0, socket) ? 0U : 1U;
          }

          startLatch.count_down();
        });

        if (!wsApp.constructorFailed())
          wsApp.run();
      });
      
      if (thread)
      {
        {
          std::scoped_lock mtx{m_threadsMux};
          m_threads.emplace_back(thread);
        }
        
        // if (!setThreadAffinity(thread->native_handle(), core))
        //   PLOGW << "Failed to assign io thread to core " << core;    
      }
      else
        PLOGF << "Failed to create I/O thread " << i;
    }

    startLatch.wait();

    if (listenSuccess != nIoThreads)
    {
      PLOGF << "Failed to listen on " << ip << ":"  << port;
      return false;
    }
    else
    {
      // #ifndef NDB_UNIT_TEST
      // m_monitor = std::move(std::jthread{[this]
      // {
      //   std::chrono::seconds period {5};
      //   std::chrono::steady_clock::time_point nextCheck = std::chrono::steady_clock::now() + period;

      //   while (m_run)
      //   {
      //     std::this_thread::sleep_for(chrono::seconds{1});

      //     if (m_run && chrono::steady_clock::now() >= nextCheck)
      //     {
      //       m_kvHandler->monitor();
      //       nextCheck = chrono::steady_clock::now() + period;
      //     }
      //   }
      // }});
      // #endif
    }
    

    #ifndef NDB_UNIT_TEST
    PLOGI << "Ready";
    #endif

    return true;
  }


private:
  std::atomic_bool m_run;
  std::shared_ptr<TsHandler> m_handler;
  std::shared_ptr<ServerStats> m_serverStats;
  std::vector<std::unique_ptr<std::jthread>> m_threads;
  std::vector<us_listen_socket_t *> m_sockets;  
  std::set<TsWebSocket *> m_wsClients;
  std::mutex m_socketsMux;
  std::mutex m_wsClientsMux;
  std::mutex m_threadsMux;
};


} // ns nemesis
} // ns core
} // ns ts


#endif
