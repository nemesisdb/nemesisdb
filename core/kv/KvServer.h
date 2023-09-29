#ifndef FC_CORE_KVSERVER_H
#define FC_CORE_KVSERVER_H


#include <functional>
#include <vector>
#include <set>
#include <tuple>
#include <latch>
#include <core/kv/KvCommon.h>
#include <core/kv/KvPoolWorker.h>
#include <core/kv/KvHandler.h>
#include <core/FusionConfig.h>


namespace fusion { namespace core { namespace kv {


class KvServer
{
public:
  KvServer() : m_fusionClose(false)
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
    m_fusionClose = true;

    {
      std::scoped_lock lck{m_sessionsMux};
      for (auto ws : m_sessions)
      {
        std::cout << "ws = " << ws << '\n';
        ws->end(1000); 
      }        

      m_sessions.clear();
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

    if (m_handlers)
      delete m_handlers;

    m_handlers = nullptr;

    if (kv::serverStats)
      delete kv::serverStats;

    kv::serverStats = nullptr;
  }


  void run (FusionConfig& config)
  {
    // cores => io threads
    static const std::map<std::size_t, std::size_t> CoresToIoThreads =
    {
      {1, 1},
      {2, 1},
      {4, 3},
      {8, 6},
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

    unsigned int maxPayload = config.cfg["kv"]["maxPayload"];
    std::string ip = config.cfg["kv"]["ip"];
    int port = config.cfg["kv"]["port"];
    

    const auto nCores = std::min<std::size_t>(std::thread::hardware_concurrency(), FUSION_MAX_CORES);

    if (nCores < 1U || nCores > 256U)
    {
      std::cout << "Core count unexpected: " << nCores << '\n';
      return;
    }
    
    const std::size_t nIoThreads = CoresToIoThreads.contains(nCores) ? CoresToIoThreads.at(nCores) : std::prev(CoresToIoThreads.upper_bound(nCores), 1)->second;
    
    kv::MaxPools = std::max<std::size_t>(1U, nCores - nIoThreads);

    m_handlers = new kv::KvHandler {kv::MaxPools, nCores - kv::MaxPools};
    
    #ifndef FC_UNIT_TEST
    std::cout << "Fusion Max Cores: " << FUSION_MAX_CORES << '\n'
              << "Available Cores: "  << std::thread::hardware_concurrency() << '\n';
    /*std::cout << "I/O Threads: "    << nIoThreads << '\n'     // TODO remove
              << "Pools: "            << kv::MaxPools << '\n';  // TODO remove */
    #endif
    
    std::size_t listenSuccess{0U};
    std::atomic_ref listenSuccessRef{listenSuccess};
    std::latch startLatch (nIoThreads);

    for (std::size_t i = 0U, core = 0U ; i < nIoThreads ; ++i, ++core)
    {
      auto * thread = new std::jthread([this, ip, port, &listenSuccessRef, &startLatch, maxPayload, serverStats = kv::serverStats]()
      {
        auto wsApp = uWS::App().ws<kv::KvSession>("/*",
        {
          .compression = uWS::DISABLED,
          .maxPayloadLength = maxPayload,
          .idleTimeout = 180, // TODO should be configurable?
          .maxBackpressure = 16 * 1024 * 1024,
          // handlers
          .upgrade = nullptr,
          
          .open = [this](kv::KvWebSocket * ws)
          {
            std::scoped_lock lck{m_sessionsMux};
            m_sessions.insert(ws);
          },
          .message = [this, serverStats](kv::KvWebSocket * ws, std::string_view message, uWS::OpCode opCode)
          {   
            ++serverStats->queryCount;

            if (opCode != uWS::OpCode::TEXT)
              ws->send(kv::createErrorResponse(kv::KvRequestStatus::OpCodeInvalid).dump(), kv::WsSendOpCode);
            else
            {
              if (auto request = kv::kvjson::parse(message, nullptr, false); request.is_discarded())
                ws->send(kv::createErrorResponse(kv::KvRequestStatus::JsonInvalid).dump(), kv::WsSendOpCode);
              else
              {
                if (auto [status, msg] = m_handlers->handle(ws, std::move(request)) ; status != kv::KvRequestStatus::Ok)
                  ws->send(kv::createErrorResponse(status, msg).dump(), kv::WsSendOpCode);
              }
            }
          },
          .close = [this](kv::KvWebSocket * ws, int /*code*/, std::string_view /*message*/)
          {
            ws->getUserData()->connected->store(false);

            // when we shutdown (stop()), have to call ws->end() other uWS doesn't return.
            // but when we call ws->end(), this lambda is called, so we need to avoid mutex deadlock with this flag
            if (!m_fusionClose)
            {
              std::scoped_lock lck{m_sessionsMux};
              m_sessions.erase(ws);
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
        m_threads.push_back(thread);
        
        if (!setThreadAffinity(thread->native_handle(), core))
          std::cout << "Failed to assign io thread to core " << core << '\n';    
      }
      else
        std::cout << "Failed to create I/O thread " << i << '\n';
    }

    startLatch.wait();

    if (listenSuccess != nIoThreads)
      std::cout << "Failed to listen on " << ip << ":"  << port << std::endl;
    

    #ifndef FC_UNIT_TEST
    std::cout << "Ready\n";
    #endif
  }

  private:
    std::vector<std::jthread *> m_threads;
    std::vector<us_listen_socket_t *> m_sockets;
    std::set<KvWebSocket *> m_sessions;
    kv::KvHandler * m_handlers;
    std::mutex m_socketsMux;
    std::mutex m_sessionsMux;
    std::atomic_bool m_fusionClose; // true if fusion has triggered a close
};


}
}
}

#endif
