#ifndef NDB_CORE_KVSERVER_H
#define NDB_CORE_KVSERVER_H


#include <functional>
#include <vector>
#include <set>
#include <tuple>
#include <latch>
#include <thread>
#include <uwebsockets/App.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvPoolWorker.h>
#include <core/kv/KvHandler.h>
#include <core/ks/KsHandler.h>  
#include <core/ks/KsSets.h>
#include <core/NemesisConfig.h>


namespace nemesis { namespace core { namespace kv {


class KvServer
{
public:
  KvServer() : m_run(true)
  {

  }


  ~KvServer()
  {
    try
    {
      stop();  
    }
    catch(const std::exception& e)
    {
      std::cerr << "Error during shutdown\n"; // don't care particularly
    }
  }


  void stop()
  {
    m_run = false;

    if (m_monitor.joinable())
      m_monitor.join();

    {
      std::scoped_lock lck{m_wsSessionsMux};
      for (auto ws : m_wsSessions)
        ws->end(1000); 

      m_wsSessions.clear();
    }

    {
      std::scoped_lock lck{m_socketsMux};

      for(auto sock : m_sockets)
        us_listen_socket_close(0, sock);

      m_sockets.clear();
    }

    for(auto thread : m_threads)
      thread->join();

    m_threads.clear();

    if (m_kvHandler)
      delete m_kvHandler;

    m_kvHandler = nullptr;

    if (kv::serverStats)
      delete kv::serverStats;

    kv::serverStats = nullptr;
  }


  void run (NemesisConfig& config)
  {
    // cores => io threads
    static const std::map<std::size_t, std::size_t> CoresToIoThreads =
    {
      {1, 1},
      {2, 1},
      {4, 3},
      {8, 6},
      {10, 8},
      {12, 10},
      {16, 12},
      {18, 15},
      {20, 17},
      {24, 20},
      {32, 28},
      {48, 44},
      {64, 58}
    };
    
    kv::serverStats = new kv::ServerStats;
    auto keySets = new ks::Sets;

    unsigned int maxPayload = config.cfg["kv"]["maxPayload"];
    std::string ip = config.cfg["kv"]["ip"];
    int port = config.cfg["kv"]["port"];
    

    const auto nCores = std::min<std::size_t>(std::thread::hardware_concurrency(), NEMESIS_MAX_CORES);

    if (nCores < 1U || nCores > 256U)
    {
      std::cout << "Core count unexpected: " << nCores << '\n';
      return;
    }
    
    const std::size_t nIoThreads = CoresToIoThreads.contains(nCores) ? CoresToIoThreads.at(nCores) : std::prev(CoresToIoThreads.upper_bound(nCores), 1)->second;
    
    kv::MaxPools = std::max<std::size_t>(1U, nCores - nIoThreads);

    m_kvHandler = new kv::KvHandler {kv::MaxPools, nCores - kv::MaxPools, keySets};
    
    #ifndef NDB_UNIT_TEST
    std::cout << "Available Cores: "  << std::thread::hardware_concurrency() << '\n'
              << "NemesisDB Max Cores: " << NEMESIS_MAX_CORES << '\n';
    //std::cout << "I/O Threads: "    << nIoThreads << '\n'
    //          << "Pools: "            << kv::MaxPools << '\n';
    #endif
    
    std::size_t listenSuccess{0U};
    std::atomic_ref listenSuccessRef{listenSuccess};
    std::latch startLatch (nIoThreads);

    for (std::size_t i = 0U, core = 0U ; i < nIoThreads ; ++i, ++core)
    {
      auto * thread = new std::jthread([this, ip, port, &listenSuccessRef, &startLatch, maxPayload, serverStats = kv::serverStats]()
      {
        auto wsApp = uWS::App().ws<WsSession>("/*",
        {
          .compression = uWS::DISABLED,
          .maxPayloadLength = maxPayload,
          .idleTimeout = 180, // TODO should be configurable?
          .maxBackpressure = 16 * 1024 * 1024,
          // handlers
          .upgrade = nullptr,
          
          .open = [this](KvWebSocket * ws)
          {
            std::scoped_lock lck{m_wsSessionsMux};
            m_wsSessions.insert(ws);
          },          
          .message = [this, serverStats](KvWebSocket * ws, std::string_view message, uWS::OpCode opCode)
          {   
            ++serverStats->queryCount;

            if (opCode != uWS::OpCode::TEXT)
              ws->send(createErrorResponse(RequestStatus::OpCodeInvalid).dump(), kv::WsSendOpCode);
            else
            {
              if (auto request = njson::parse(message, nullptr, false); request.is_discarded()) // TODO change parse() to accept()?
                ws->send(createErrorResponse(RequestStatus::JsonInvalid).dump(), kv::WsSendOpCode);
              else
              {
                const auto& commandName = request.cbegin().key();

                if (request.size() != 1U)
                  ws->send(createErrorResponse(commandName + "_RSP", RequestStatus::CommandMultiple).dump(), kv::WsSendOpCode);
                else
                {
                  auto [status, queryName] = m_kvHandler->handle(ws, std::move(request));

                  if (status == RequestStatus::CommandType)
                    ws->send(createErrorResponse(queryName+"_RSP", status).dump(), kv::WsSendOpCode);
                  else if (status != RequestStatus::Ok)
                    ws->send(createErrorResponse(status, queryName).dump(), kv::WsSendOpCode);
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
            {
              std::scoped_lock lck{m_wsSessionsMux};
              m_wsSessions.erase(ws);
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
          m_threads.push_back(thread);
        }
        
        if (!setThreadAffinity(thread->native_handle(), core))
          std::cout << "Failed to assign io thread to core " << core << '\n';    
      }
      else
        std::cout << "Failed to create I/O thread " << i << '\n';
    }

    startLatch.wait();

    if (listenSuccess != nIoThreads)
      std::cout << "Failed to listen on " << ip << ":"  << port << std::endl;
    else
    {
      #ifndef NDB_UNIT_TEST_NOMONITOR
      m_monitor = std::move(std::jthread{[this]
      {
        std::chrono::seconds period {5};
        std::chrono::steady_clock::time_point nextCheck = std::chrono::steady_clock::now() + period;

        while (m_run)
        {
          std::this_thread::sleep_for(std::chrono::seconds{1});

          if (m_run && std::chrono::steady_clock::now() >= nextCheck)
          {
            m_kvHandler->monitor();
            nextCheck = std::chrono::steady_clock::now() + period;
          }
        }        
      }});
      #endif
    }
    

    #ifndef NDB_UNIT_TEST
    std::cout << "Ready\n";
    #endif
  }


  private:
    std::vector<std::jthread *> m_threads;
    std::vector<us_listen_socket_t *> m_sockets;
    std::set<KvWebSocket *> m_wsSessions;
    kv::KvHandler * m_kvHandler;
    std::mutex m_socketsMux;
    std::mutex m_wsSessionsMux;
    std::mutex m_threadsMux;
    std::atomic_bool m_run; // true if fusion has triggered a close
    std::jthread m_monitor;
};


}
}
}

#endif
