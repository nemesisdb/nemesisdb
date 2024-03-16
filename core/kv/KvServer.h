#ifndef NDB_CORE_KVSERVER_H
#define NDB_CORE_KVSERVER_H


//#include <dirent.h>     // for checking /proc
//#include <arpa/inet.h>  // for inet_pton() etc
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
      PLOGF << "Error during shutdown";
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

    // TODO loading/saving
    // if (config.load())
    // {
    //   if (auto [ok, msg] = load(config); !ok)
    //   {
    //     PLOGF << msg;
    //     return false;
    //   }
    // }

    unsigned int maxPayload = config.cfg["kv"]["maxPayload"].as<unsigned int>();
    std::string ip = config.cfg["kv"]["ip"].as_string();
    int port = config.cfg["kv"]["port"].as<unsigned int>();


    std::size_t listenSuccess{0U};
    std::atomic_ref listenSuccessRef{listenSuccess};
    std::latch startLatch (1);
    std::size_t core = 0;

    
    m_thread.reset(new std::jthread([this, ip, port, &listenSuccessRef, &startLatch, maxPayload, serverStats = kv::serverStats]()
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
              // TODO look at pmr parse: https://github.com/danielaparker/jsoncons/blob/master/doc/Examples.md#parse-a-json-text-using-a-polymorphic_allocator-since-01710
              request = njson::parse(message);
            }
            catch (...)
            {
              // handled below, would have to check request.is_null() anyway
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
    }));
    

    if (m_thread)
    { 
      if (!setThreadAffinity(m_thread->native_handle(), core))
        PLOGW << "Failed to assign io thread to core " << core;    
    }
    else
      PLOGF << "Failed to create I/O thread";

    startLatch.wait();

    if (listenSuccess != nIoThreads)
    {
      PLOGF << "Failed to listen on " << ip << ":"  << port;
      return false;
    }
    else
    {
      #ifndef NDB_UNIT_TEST
      // m_monitor = std::move(std::jthread{[this]
      // {
      //   std::chrono::seconds period {5};
      //   std::chrono::steady_clock::time_point nextCheck = std::chrono::steady_clock::now() + period;

      //   while (m_run)
      //   {
      //     std::this_thread::sleep_for(std::chrono::seconds{1});

      //     if (m_run && std::chrono::steady_clock::now() >= nextCheck)
      //     {
      //       m_kvHandler->monitor();
      //       nextCheck = std::chrono::steady_clock::now() + period;
      //     }
      //   }
      // }});
      #endif
    }
    

    #ifndef NDB_UNIT_TEST
    PLOGI << "Ready";
    #endif

    return true;
  }

  
  private:

    std::tuple<bool, std::size_t> init(const njson& config)
    {
      kv::serverStats = new kv::ServerStats;
      m_kvHandler = new kv::KvHandler {config};
      return {true, 1};
     

      // if (NemesisConfig::kvSaveEnabled(config))
      // {
      //   // test we can write to the kv save path
      //   if (std::filesystem::path path {NemesisConfig::kvSavePath(config)}; !std::filesystem::exists(path) || !std::filesystem::is_directory(path))
      //   {
      //     PLOGF << "kv:session::save::path is not a directory or does not exist";
      //     return {false, 0};
      //   }
      //   else
      //   {
      //     const auto filename = createUuid();
      //     std::filesystem::path fullPath{path};
      //     fullPath /= filename;

      //     if (std::ofstream out{fullPath}; !out.good())
      //     {
      //       PLOGF << "Cannot write to kv:session::save::path";
      //       return {false, 0};
      //     }
      //     else
      //     {
      //       out.close();
      //       std::filesystem::remove(fullPath);
      //     }
      //   }
      // }
    }
    
    
    /*
    std::tuple<bool, std::string> load(const NemesisConfig& config)
    {
      fs::path root = config.loadPath / config.loadName;

      if (!fs::exists(root))
        return {false, "Load name does not exist"};
      
      if (!fs::is_directory(root))
        return {false, "Path to load name is not a directory"};

      std::size_t max = 0;

      for (auto& dir : fs::directory_iterator(root))
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

      PLOGI << "Reading metadata in " << md;
      
      std::ifstream mdStream {md / "md.json"};
      auto mdJson = njson::parse(mdStream);

      if (!(mdJson.contains("status") && mdJson.contains("pools")) || !(mdJson["status"].is_uint64() && mdJson["pools"].is_uint64()))
        return {false, "Metadata file invalid"};
      else if (mdJson["status"] != toUnderlying(KvSaveStatus::Complete))
        return {false, "Dataset is not complete, cannot load. Metadata status not Complete"};
      else
      {
        const auto loadResult = m_kvHandler->load(data);
        const auto success = LoadResult::statusSuccess(loadResult);

        std::cout << "-- Load --\n";
    
        if (success)
        {
          std::cout << "Status: Success " << (loadResult.status == RequestStatus::LoadDuplicate ? "(Duplicates)" : "") << '\n';
          std::cout << "Sessions: " << loadResult.nSessions << "\nKeys: " << loadResult.nKeys << '\n'; 
        }
        else
          std::cout << "Status: Fail\n";

        std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(loadResult.loadTime).count() << '\n';

        std::cout << "----------\n";
        
        return {success, ""};
      }
    }
    */

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
    std::unique_ptr<std::jthread> m_thread;
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
