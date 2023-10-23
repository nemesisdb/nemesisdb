#include <signal.h> 
#include <iostream>
#include <thread>
#include <algorithm>
#include <mutex>
#include <latch>
#include <filesystem>
#include <core/NemesisConfig.h>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvHandler.h>
#include <core/kv/KvServer.h>


using namespace nemesis::core;
namespace kv = nemesis::core::kv;

std::latch run{1U};


inline void kvSigHandle(int param)
{
  run.count_down();
}



int main (int argc, char ** argv)
{
  std::cout << "NemesisDB v" << NEMESIS_VERSION << " starting\n";

  std::cout << "Registering signals\n";
  signal(SIGINT,  kvSigHandle);
  signal(SIGTERM, kvSigHandle); // docker stop
  signal(SIGKILL, kvSigHandle); // docker kill 


  NemesisConfig config;

  #ifndef NDEBUG
    config.cfg["version"] = 1;
    config.cfg["kv"]["ip"] = "127.0.0.1";
    config.cfg["kv"]["port"] = 1987;
    config.cfg["kv"]["maxPayload"] = 1024U;
  #else
    if (nemesis::core::readConfig(config, argc, argv); !config.valid)
      return 0;
  #endif

  kv::KvServer server;
  server.run(config);

  run.wait();

  server.stop();
}

