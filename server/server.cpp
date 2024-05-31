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
#include <core/ts/TsServer.h>
#include <core/LogFormatter.h>
#include <jsoncons/json_traits_macros.hpp>



using namespace nemesis::core;

namespace kv = nemesis::core::kv;
namespace ts = nemesis::core::ts;

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
    config.cfg["version"] = 1;
    config.cfg["mode"] = "kv_sessions";
    config.cfg["core"] = 0;

    config.cfg["ip"] = "127.0.0.1";
    config.cfg["port"] = 1987;
    config.cfg["maxPayload"] = 2048;

    config.cfg["kv"]["save"]["enabled"] = false;
    config.cfg["kv"]["save"]["path"] = "./data";

    config.cfg["kv_sessions"]["save"]["enabled"] = false;
    config.cfg["kv_sessions"]["save"]["path"] = "./data";

    config.cfg["ts"]["ip"] = "127.0.0.1";
    config.cfg["ts"]["port"] = 1987;

    if (NemesisConfig::serverMode(config.cfg) == ServerMode::KvSessions && !fs::exists(config.cfg["kv_sessions"]["save"]["path"].as_string()))
      fs::create_directories(config.cfg["kv_sessions"]["save"]["path"].as_string());
    else if (NemesisConfig::serverMode(config.cfg) == ServerMode::KV && !fs::exists(config.cfg["kv"]["save"]["path"].as_string()))
      fs::create_directories(config.cfg["kv"]["save"]["path"].as_string());

    // config.loadPath = NemesisConfig::savePath(config.cfg);
    // config.loadName = "t2";
      
  #else
    if (config = nemesis::core::readConfig(argc, argv); !config.valid)
      return 1;
  #endif

  
  int error = 0;

  auto runServer = [&config, &error]<typename Server> (Server&& server)
  {
    if (server.run(config))
      run.wait();
    else
    {
      error = 1;
      run.count_down();
    }
  
    server.stop();
  };
  

  PLOGI_IF (NemesisConfig::saveEnabled(config.cfg))  << "Save: Enabled (" << NemesisConfig::savePath(config.cfg) << ')';
  PLOGI_IF (!NemesisConfig::saveEnabled(config.cfg)) << "Save: Disabled";

  switch (NemesisConfig::serverMode(config.cfg))
  {
    case ServerMode::KV:
      PLOGI << "Mode: KV";
      runServer (kv::KvServer{});
    break;

    case ServerMode::KvSessions:
      PLOGI << "Mode: KV Sessions";
      runServer (kv::KvSessionServer{});
    break;

    case ServerMode::TS:
      PLOGI << "Mode: TS";
      runServer (ts::TsServer{});
    break;

    default:
      // caught by readConfig()
    break;
  }
  
  return error;
}
