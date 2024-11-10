#include <signal.h> 
#include <iostream>
#include <thread>
#include <algorithm>
#include <latch>
#include <filesystem>
#include <core/NemesisConfig.h>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvServer.h>
#include <core/LogFormatter.h>
#include <jsoncons/json_traits_macros.hpp>


using namespace nemesis::core;

namespace kv = nemesis::core::kv;

static std::latch run{1U};
static plog::ColorConsoleAppender<NdbFormatter> consoleAppender;


inline void kvSigHandle(int param)
{
  run.count_down();
}


int main (int argc, char ** argv)
{
  initLogger(consoleAppender); // keep this as first statement of main()
  
  PLOGI << "NemesisDB v" << NEMESIS_VERSION << " starting";
  PLOGI << "Registering signals";

  signal(SIGINT,  kvSigHandle);
  signal(SIGTERM, kvSigHandle); // docker stop
  signal(SIGKILL, kvSigHandle); // docker kill 


  NemesisConfig config;

  #ifdef NDB_DEBUG
    config.cfg["version"] = 5;
    config.cfg["sessionsEnabled"] = false;
    config.cfg["core"] = 0;

    config.cfg["ip"] = "127.0.0.1";
    config.cfg["port"] = 1987;
    config.cfg["maxPayload"] = 2048;

    config.cfg["kv"]["save"]["enabled"] = false;
    config.cfg["kv"]["save"]["path"] = "./data";

    config.cfg["sessions"]["save"]["enabled"] = false;
    config.cfg["sessions"]["save"]["path"] = "./data";

    if (NemesisConfig::serverMode(config.cfg) == ServerMode::KvSessions && !fs::exists(config.cfg["sessions"]["save"]["path"].as_string()))
      fs::create_directories(config.cfg["sessions"]["save"]["path"].as_string());
    else if (NemesisConfig::serverMode(config.cfg) == ServerMode::KV && !fs::exists(config.cfg["kv"]["save"]["path"].as_string()))
      fs::create_directories(config.cfg["kv"]["save"]["path"].as_string());

    // config.loadPath = NemesisConfig::savePath(config.cfg);
    // config.loadName = "t2";
      
  #else
    if (config = nemesis::core::readConfig(argc, argv); !config.valid)
      return 1;
  #endif


  int error = 0;

  const bool save = NemesisConfig::saveEnabled(config.cfg);
  const std::string address = NemesisConfig::wsSettings(config.cfg).ip + ":" + std::to_string(NemesisConfig::wsSettings(config.cfg).port);

  auto runServer = [&config, &error, save, address]<typename Server> (Server&& server, const bool sessions)
  {
    PLOGI_IF (save)  << "Save: Enabled (" << NemesisConfig::savePath(config.cfg) << ')';
    PLOGI_IF (save) << "Save: Disabled";
    PLOGI << "Sessions Enabled: " << (sessions ? "yes" : "no");
    PLOGI << "Address: " << address;
    
    if (server.run(config))
      run.wait();
    else
    {
      error = 1;
      run.count_down();
    }
  
    server.stop();
  };
  
  switch (NemesisConfig::serverMode(config.cfg))
  {
    case ServerMode::KV:
      runServer (kv::KvServer{}, false);
    break;

    case ServerMode::KvSessions:
      runServer (kv::KvSessionServer{}, true);
    break;

    default:
      // caught by readConfig()
    break;
  }
  
  return error;
}
