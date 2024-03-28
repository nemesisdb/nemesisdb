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
    config.cfg["mode"] = "kv";

    config.cfg["kv"]["ip"] = "127.0.0.1";
    config.cfg["kv"]["port"] = 1987;
    config.cfg["kv"]["maxPayload"] = 2048;
    config.cfg["kv"]["sessions"]["enabled"] = false;
    config.cfg["kv"]["sessions"]["save"]["enabled"] = true;
    config.cfg["kv"]["sessions"]["save"]["path"] = "./data";

    config.cfg["ts"]["ip"] = "127.0.0.1";
    config.cfg["ts"]["port"] = 1987;
    config.cfg["ts"]["maxPayload"] = 2048;

    if (config.cfg["kv"]["sessions"]["enabled"] == true && !fs::exists(config.cfg["kv"]["sessions"]["save"]["path"].as_string()))
      fs::create_directories(config.cfg["kv"]["sessions"]["save"]["path"].as_string());

    // config.loadPath = NemesisConfig::kvSavePath(config.cfg);
    // config.loadName = "t2";
      
  #else
    if (config = nemesis::core::readConfig(argc, argv); !config.valid)
      return 1;
  #endif

  
  int error = 0;

  if (config.serverMode() == ServerMode::KV)
  {
    PLOGI << "Mode: KV";

    if (NemesisConfig::haveSessions(config.cfg))
    {
      PLOGI << "Sessions: Enabled";

      kv::KvSessionServer server;

      if (server.run(config))
        run.wait();
      else
      {
        error = 1;
        run.count_down();
      }
    
      server.stop();
    }      
    else
    {
      PLOGI << "Sessions: Disabled";

      kv::KvServer server;

      if (server.run(config))
        run.wait();
      else
      {
        error = 1;
        run.count_down();
      }
    
      server.stop();
    }
  }
  else if (config.serverMode() == ServerMode::TS)
  {
    PLOGI << "Mode: TS";

    ts::TsServer tsServer;
    
    if (tsServer.run(config))
      run.wait();
    else
    {
      error = 1;
      run.count_down();
    }
  }
  else
  {
    // caught by readConfig()
  }
  
  
  return error;
}
