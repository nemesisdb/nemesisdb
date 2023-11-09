#ifndef NDB_CORE_KVSERVER_H
#define NDB_CORE_KVSERVER_H


#include <arpa/inet.h>  // for inet_pton() etc
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
#include <core/NemesisConfig.h>


namespace nemesis { namespace core { namespace kv {


class KvServer
{
public:
  KvServer() : m_run(true), m_kvHandler(nullptr)
  {
    kv::serverStats = nullptr;
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


  bool run (NemesisConfig& config)
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
    
    

    const auto nCores = std::min<std::size_t>(std::thread::hardware_concurrency(), NEMESIS_MAX_CORES);

    if (nCores < 1U || nCores > 64U)
    {
      std::cout << "Core count unexpected: " << nCores << '\n';
      return false;
    }
    

    unsigned int maxPayload = config.cfg["kv"]["maxPayload"].as<unsigned int>();
    std::string ip = config.cfg["kv"]["ip"].as_string();
    int port = config.cfg["kv"]["port"].as<unsigned int>();

    if (auto check = isPortOpen(ip, port); !check || *check)
    {
      std::cout << "ERROR: IP and port already used OR failed during checking open ports. " << ip << ":" << port << '\n';
      return false;
    }


    kv::serverStats = new kv::ServerStats;

    const std::size_t nIoThreads = CoresToIoThreads.contains(nCores) ? CoresToIoThreads.at(nCores) : std::prev(CoresToIoThreads.upper_bound(nCores), 1)->second;
    
    kv::MaxPools = std::max<std::size_t>(1U, nCores - nIoThreads);

    m_kvHandler = new kv::KvHandler {kv::MaxPools, nCores - kv::MaxPools};
    
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
              ws->send(createErrorResponse(RequestStatus::OpCodeInvalid).to_string(), kv::WsSendOpCode);
            else
            {
              // TODO this avoids exception on parse error, but not clear how to create json object
              //      from reader output (if even possible)
              // jsoncons::json_string_reader reader(message);
              // std::error_code ec;
              // reader.read(ec);

              njson request = njson::null();

              try
              {
                request = std::move(njson::parse(message));
              }
              catch (...)
              {

              }
              
              if (request.is_null())
                ws->send(createErrorResponse(RequestStatus::JsonInvalid).to_string(), kv::WsSendOpCode);
              else if (request.empty() || !request.is_object()) //i.e. top level not an object
                ws->send(createErrorResponse(RequestStatus::CommandSyntax).to_string(), kv::WsSendOpCode);
              else if (request.size() > 1U)
                ws->send(createErrorResponse(RequestStatus::CommandMultiple).to_string(), kv::WsSendOpCode);
              else
              {
                const auto& commandName = request.object_range().cbegin()->key();

                if (!request.at(commandName).is_object())
                  ws->send(createErrorResponse(commandName+"_RSP", RequestStatus::CommandType).to_string(), kv::WsSendOpCode);
                else if (const auto status = m_kvHandler->handle(ws, commandName, std::move(request)); status != RequestStatus::Ok)
                  ws->send(createErrorResponse(commandName+"_RSP", status).to_string(), kv::WsSendOpCode);
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
    {
      std::cout << "Failed to listen on " << ip << ":"  << port << std::endl;
      return false;
    }
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

    return true;
  }

  private:

    std::optional<bool> isPortOpen (const std::string& checkIp, const int checkPort)
    {
      #ifndef NDB_UNIT_TEST_NOPORTCHECK

      if (auto fd = socket(AF_INET, SOCK_STREAM, 0) ; fd >= 0)
      {        
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = checkPort;

        inet_pton(AF_INET, checkIp.c_str(), &(address.sin_addr));
    
        return bind(fd, (struct sockaddr*)&address,sizeof(address)) < 0;
      }
 
      return {};
      #else
      return false;
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
    std::atomic_bool m_run;
    std::jthread m_monitor;
};


}
}
}

#endif
