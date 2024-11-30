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


  Settings settings;

  #ifdef NDB_DEBUG
    const auto s = R"({
                        "version":5,                
                        "core":0,                   
                        "ip":"127.0.0.1",           
                        "port":1987,
                        "maxPayload":1024,          
                        "persist":
                        {
                          "enabled":true,          
                          "path":"./data"
                        }
                      })";

    settings = Settings{njson::parse(s)};

    if (settings.persistEnabled && !fs::exists(settings.persistPath))
      fs::create_directories(settings.persistPath);
      
  #else
    PLOGI << "Reading config";

    if (settings = nemesis::readConfig(argc, argv); !settings.valid)
      return 1;
  #endif


  if (settings.persistEnabled)
    PLOGI << "Persist: Enabled (" << settings.persistPath << ')';
  else
    PLOGI << "Persist: Disabled";
  
  PLOGI << "Query Interface: " << settings.wsSettingsString();
  

  int error = 0;
  auto sessions = std::make_shared<nemesis::sh::Sessions>();

  Server server;
  if (server.run(settings, sessions))
    run.wait();
  else
  {
    error = 1;
    run.count_down();
  }

  server.stop();

  return error;
}
