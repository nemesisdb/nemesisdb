#include <signal.h> 
#include <iostream>
#include <thread>
#include <algorithm>
#include <latch>
#include <filesystem>
#include <core/Server.h>
#include <core/NemesisConfig.h>
#include <core/NemesisCommon.h>
#include <core/LogFormatter.h>
#include <core/kv/KvCommon.h>
#include <core/sh/ShSessions.h>
#include <jsoncons/json_traits_macros.hpp>


using namespace nemesis;


static std::latch run{1U};
static plog::ColorConsoleAppender<NdbFormatter> consoleAppender;


inline void kvSigHandle(int param)
{
  run.count_down();
}


int main (int argc, char ** argv)
{
  initLogger(consoleAppender); // always first statement of main()
  
  PLOGI << "NemesisDB v" << NEMESIS_VERSION << " starting";
  PLOGI << "Registering signals";

  signal(SIGINT,  kvSigHandle);
  signal(SIGTERM, kvSigHandle); // docker stop
  signal(SIGKILL, kvSigHandle); // docker kill 


  NemesisConfig config;

  #ifdef NDB_DEBUG
    config.cfg["version"] = 5;
    //config.cfg["sessionsEnabled"] = true;
    config.cfg["core"] = 0;

    config.cfg["ip"] = "127.0.0.1";
    config.cfg["port"] = 1987;
    config.cfg["maxPayload"] = 2048;

    config.cfg["persist"]["enabled"] = true;
    config.cfg["persist"]["path"] = "./data";

    if (config.cfg["persist"]["enabled"] == true && !fs::exists(config.cfg["persist"]["path"].as_string()))
      fs::create_directories(config.cfg["persist"]["path"].as_string());

    // config.loadPath = NemesisConfig::savePath(config.cfg);
    // config.loadName = "t2";
      
  #else
    if (config = nemesis::core::readConfig(argc, argv); !config.valid)
      return 1;
  #endif


  if (NemesisConfig::persistEnabled(config.cfg))
    PLOGI << "Persist: Enabled (" << NemesisConfig::savePath(config.cfg) << ')';
  else
    PLOGI << "Persist: Disabled";
  
  PLOGI << "Query Interface: " << NemesisConfig::wsSettingsString(config.cfg);
  

  int error = 0;
  auto sessions = std::make_shared<nemesis::sh::Sessions>();

  Server server;
  if (server.run(config, sessions))
    run.wait();
  else
  {
    error = 1;
    run.count_down();
  }

  server.stop();

  return error;
}
