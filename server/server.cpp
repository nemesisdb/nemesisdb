#include <iostream>
#include <thread>
#include <algorithm>
#include <mutex>
#include <latch>
#include <filesystem>
#include <uwebsockets/App.h>
#include <core/FusionConfig.h>
#include <core/FusionCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvHandler.h>
#include <core/kv/KvServer.h>


using namespace fusion::core;
namespace kv = fusion::core::kv;


int main (int argc, char ** argv)
{
  std::cout << "Fusion v" << FUSION_VERSION << " starting\n";

  FusionConfig config;

  #ifndef NDEBUG
    config.cfg["version"] = 1;
    config.cfg["kv"]["ip"] = "127.0.0.1";
    config.cfg["kv"]["port"] = 1987;
    config.cfg["kv"]["maxPayload"] = 1024U;
  #else
    if (fusion::core::readConfig(config, argc, argv); !config.valid)
      return 0;
  #endif


  kv::KvServer server;
  server.run(config);  
}

