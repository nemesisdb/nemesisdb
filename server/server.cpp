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
#include <plog/Init.h>
#include <plog/Appenders/ColorConsoleAppender.h>

using namespace nemesis::core;
namespace kv = nemesis::core::kv;

static std::latch run{1U};
static plog::ColorConsoleAppender<NdbFormatter> consoleAppender;

inline void kvSigHandle(int param)
{
  run.count_down();
}

//JSONCONS_ENUM_TRAITS(kv::RequestStatus, Ok); // TODO

int main (int argc, char ** argv)
{
  // keep this as first statement of main()
  #ifdef NDB_DEBUG
    plog::init(plog::verbose, &consoleAppender);
  #else
    plog::init(plog::info, &consoleAppender);    
  #endif
  
  PLOGI << "NemesisDB v" << NEMESIS_VERSION << " starting";
  PLOGI << "Registering signals";

  signal(SIGINT,  kvSigHandle);
  signal(SIGTERM, kvSigHandle); // docker stop
  signal(SIGKILL, kvSigHandle); // docker kill 


  NemesisConfig config;

  #ifdef NDB_DEBUG
    config.cfg["version"] = 1;
    config.cfg["kv"]["ip"] = "127.0.0.1";
    config.cfg["kv"]["port"] = 1987;
    config.cfg["kv"]["maxPayload"] = 2048;
    config.cfg["session"]["save"]["enabled"] = true;
    config.cfg["session"]["save"]["path"] = "./data";

    if (!fs::exists(config.cfg["session"]["save"]["path"].as_string()))
      fs::create_directories(config.cfg["session"]["save"]["path"].as_string());
      
  #else
    if (config = nemesis::core::readConfig(argc, argv); !config.valid)
      return 0;
  #endif

  kv::KvServer server;
  
  if (server.run(config))
    run.wait();
  else
    run.count_down();

  server.stop();
}
