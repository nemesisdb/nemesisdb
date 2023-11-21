#ifndef NDB_CORE_KVSERVER_H
#define NDB_CORE_KVSERVER_H


#include <dirent.h>     // for checking /proc
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

namespace fs = std::filesystem;


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
      std::cout << "Error during shutdown\n"; // don't care particularly
    }
  }


  void stop()
  {
    m_run = false;

    if (m_monitor.joinable())
      m_monitor.join();

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
    std::size_t nIoThreads = 0 ;
    bool started = false;

    if (std::tie(started, nIoThreads) = init(config.cfg); !started)
      return false;

    if (config.load())
    {
      if (auto [ok, msg] = load(config); !ok)
      {
        std::cout << msg << '\n';
        return false;
      }
    }

    unsigned int maxPayload = config.cfg["kv"]["maxPayload"].as<unsigned int>();
    std::string ip = config.cfg["kv"]["ip"].as_string();
    int port = config.cfg["kv"]["port"].as<unsigned int>();


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
          .maxBackpressure = 24 * 1024 * 1024,
          // handlers
          .upgrade = nullptr,
          
          .open = [this](KvWebSocket * ws)
          {
            std::scoped_lock lck{m_wsClientsMux};
            m_wsClients.insert(ws);
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

    std::tuple<bool, std::size_t> init(const njson& config)
    {
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
      

      if (NemesisConfig::kvSaveEnabled(config))
      {
        // test we can write to the kv save path
        if (std::filesystem::path path {NemesisConfig::kvSavePath(config)}; !std::filesystem::exists(path) || !std::filesystem::is_directory(path))
        {
          std::cout << "session::save::path is not a directory or does not exist\n";
          return {false, 0};
        }
        else
        {
          auto filename = createUuid();
          std::filesystem::path fullPath{path};
          fullPath /= filename;

          if (std::ofstream out{fullPath}; !out.good())
          {
            std::cout << "Cannot write to session::save::path\n";
            return {false, 0};
          }
          else
          {
            out.close();
            std::filesystem::remove(fullPath);
          }
        }
      }


      if (const auto nCores = std::min<std::size_t>(std::thread::hardware_concurrency(), NEMESIS_MAX_CORES); nCores < 1U || nCores > 64U)
      {
        std::cout << "Core count unexpected: " << nCores << '\n';
        return {false, 0};
      }
      else
      {
        const std::size_t nIoThreads = CoresToIoThreads.contains(nCores) ? CoresToIoThreads.at(nCores) : std::prev(CoresToIoThreads.upper_bound(nCores), 1)->second;
        
        kv::MaxPools = std::max<std::size_t>(1U, nCores - nIoThreads);
        kv::serverStats = new kv::ServerStats;
        
        m_kvHandler = new kv::KvHandler {kv::MaxPools, nCores - kv::MaxPools, config};
      
        return {true, nIoThreads};
      }

      /* TODO decide: when we shutdown, client connections enter TIME_WAIT, preventing starting
      //              if isPortOpen() is called. 
      //              See: https://www.baeldung.com/linux/close-socket-time_wait
      if (auto check = isPortOpen(ip, port); !check || *check)
      {
        std::cout << "ERROR: IP and port already used OR failed during checking open ports. " << ip << ":" << port << '\n';
        return false;
      }
      */
    }


    std::tuple<bool, std::string> load(const NemesisConfig& config)
    {
      fs::path root = config.loadPath / config.loadName;

      if (!fs::exists(root))
        return {false, "Load name does not exist"};
      
      if (!fs::is_directory(root))
        return {false, "Path to load name is not a directory"};

      std::size_t max = 0;

      for (auto& dir : fs::directory_iterator(root, fs::directory_options::follow_directory_symlink))
      {
        if (dir.is_directory())
          max = std::max<std::size_t>(std::stoul(dir.path().filename()), max);
      }

      if (!max)
        return {false, "No data"};

      const fs::path datasets = root / std::to_string(max);
      const fs::path data = datasets / "data";
      const fs::path md = datasets / "md";

      if (!(fs::exists(data) && fs::is_directory(data)))
        return {false, "'data' directory does not exist or is not a directory"};
      
      if (!(fs::exists(md) && fs::is_directory(md)))
        return {false, "'md' directory does not exist or is not a directory"};

      std::cout << "Reading metadata in " << md << "\n";
      
      std::ifstream mdStream {md / "md.json"};
      auto mdJson = njson::parse(mdStream);

      if (!(mdJson.contains("status") && mdJson.contains("pools")) || !(mdJson["status"].is_uint64() && mdJson["pools"].is_uint64()))
        return {false, "Metadata file invalid"};
      else if (mdJson["status"] == toUnderlying(KvSaveStatus::Complete))
      {
        m_kvHandler->loadOnStartUp(data);
        return {true, ""};
      }
      else
        return {false, "Dataset is not complete, cannot load. Metadata status not Complete"};
    }


    /*
    std::optional<bool> isPortOpen (const std::string& checkIp, const short checkPort)
    {
      #ifdef NDB_UNIT_TEST_NOPORTCHECK
      return false;
      #else
      
      if (auto fd = socket(AF_INET, SOCK_STREAM, 0) ; fd >= 0)
      {        
        sockaddr a;
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_port = htons(checkPort);

        inet_pton(AF_INET, checkIp.c_str(), &(address.sin_addr));
    
        const auto open = bind(fd, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0;
        
        if (open)
        {
          char msg[256];
          std::cout << strerror_r(errno, msg, 256) << '\n';
        }

        close(fd);

        return open;
      }
      return {};

      #endif
    }

    int getPid(const std::string& procName)
    {
      int pid = -1;

      // Open the /proc directory
      DIR *dp = opendir("/proc");
      if (dp != NULL)
      {
        // Enumerate all entries in directory until process found
        struct dirent *dirp;
        while (pid < 0 && (dirp = readdir(dp)))
        {
          // Skip non-numeric entries
          int id = atoi(dirp->d_name);
          if (id > 0)
          {
            // Read contents of virtual /proc/{pid}/cmdline file
            std::string cmdPath = std::string("/proc/") + dirp->d_name + "/cmdline";
            std::ifstream cmdFile(cmdPath.c_str());
            std::string cmdLine;
            if (std::getline(cmdFile, cmdLine); !cmdLine.empty())
            {
              // Keep first cmdline item which contains the program path
              size_t pos = cmdLine.find('\0');
              if (pos != std::string::npos)
                cmdLine = cmdLine.substr(0, pos);
              // Keep program name only, removing the path
              pos = cmdLine.rfind('/');
              if (pos != std::string::npos)
                cmdLine = cmdLine.substr(pos + 1);

              // Compare against requested process name
              if (procName == cmdLine)
                pid = id;
            }
          }
        }
      }

      closedir(dp);

      return pid;
    }
    */


  private:
    std::vector<std::jthread *> m_threads;
    std::vector<us_listen_socket_t *> m_sockets;
    std::set<KvWebSocket *> m_wsClients;
    kv::KvHandler * m_kvHandler;
    std::mutex m_socketsMux;
    std::mutex m_wsClientsMux;
    std::mutex m_threadsMux;
    std::atomic_bool m_run;
    std::jthread m_monitor;
};


}
}
}

#endif
