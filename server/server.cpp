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


using namespace fusion::core;
namespace kv = fusion::core::kv;


int main (int argc, char ** argv)
{
  // cores => io threads
  static const std::map<std::size_t, std::size_t> CoresToIoThreads =
  {
    {1, 1},
    {2, 1},
    {4, 3},
    {8, 6},
    {12, 10},
    {16, 12},
    {18, 15},
    {20, 17},
    {24, 20},
    {32, 28},
    {48, 44},
    {64, 58}
  };

  std::cout << "Fusion v" << FUSION_VERSION << " starting\n";

  #ifndef NDEBUG
    config.cfg["version"] = 1;
    config.cfg["kv"]["ip"] = "127.0.0.1";
    config.cfg["kv"]["port"] = 1987;
    config.cfg["kv"]["maxPayload"] = 1024U;
  #else
    if (fusion::core::readConfig(argc, argv); !config.valid)
      return 0;
  #endif

  kv::serverStats = new kv::ServerStats;

  unsigned int maxPayload = config.cfg["kv"]["maxPayload"];
  std::string ip = config.cfg["kv"]["ip"];
  int port = config.cfg["kv"]["port"];
  

  const auto nCores = std::min<std::size_t>(std::thread::hardware_concurrency(), FUSION_MAX_CORES);

  if (nCores < 1U || nCores > 256U)
  {
    std::cout << "Core count unexpected: " << nCores << '\n';
    return 0;
  }
  
  const std::size_t nIoThreads = CoresToIoThreads.contains(nCores) ? CoresToIoThreads.at(nCores) : std::prev(CoresToIoThreads.upper_bound(nCores), 1)->second;
  
  kv::MaxPools = std::max<std::size_t>(1U, nCores - nIoThreads);

  kv::KvHandler handlers {kv::MaxPools, nCores - kv::MaxPools};
  std::vector<std::jthread *> threads;

  std::cout << "Fusion Max Cores: " << FUSION_MAX_CORES << '\n'
            << "Available Cores: "  << std::thread::hardware_concurrency() << '\n';

  /*std::cout << "I/O Threads: "      << nIoThreads << '\n'      // TODO remove
            << "Pools: "            << kv::MaxPools << '\n';   // TODO remove*/

  std::size_t listenSuccess{0U};
  std::atomic_ref listenSuccessRef{listenSuccess};
  std::latch startLatch (nIoThreads);

  for (std::size_t i = 0U, core = 0U ; i < nIoThreads ; ++i, ++core)
  {
    auto * thread = new std::jthread([&handlers, &ip, port, &listenSuccessRef, &startLatch, maxPayload, serverStats = kv::serverStats]()
    {
      auto wsApp = uWS::App().ws<kv::KvSession>("/*",
      {
        .compression = uWS::DISABLED,
        .maxPayloadLength = maxPayload,
        .idleTimeout = 180, // TODO should be configurable?
        .maxBackpressure = 16 * 1024 * 1024,
        // handlers
        .upgrade = nullptr,
        .open = [](kv::KvWebSocket * ws)
        {
        },
        .message = [&handlers, serverStats](kv::KvWebSocket * ws, std::string_view message, uWS::OpCode opCode)
        {   
          ++serverStats->queryCount;

          if (opCode != uWS::OpCode::TEXT)
            ws->send(kv::createErrorResponse(kv::KvRequestStatus::OpCodeInvalid).dump(), kv::WsSendOpCode);
          else
          {
            if (auto request = kv::kvjson::parse(message, nullptr, false); request.is_discarded())
              ws->send(kv::createErrorResponse(kv::KvRequestStatus::JsonInvalid).dump(), kv::WsSendOpCode);
            else
            {
              //ws->getUserData()->json = std::move(request);
              
              if (auto [status, msg] = handlers.handle(ws, std::move(request)) ; status != kv::KvRequestStatus::Ok)
                ws->send(kv::createErrorResponse(status, msg).dump(), kv::WsSendOpCode);
            }
          }
        },
        .close = [](kv::KvWebSocket * ws, int /*code*/, std::string_view /*message*/)
        {
          ws->getUserData()->connected->store(false);
        }
      })
      .listen(ip, port, [port, &listenSuccessRef, &startLatch](auto * listenSocket)
      {
        if (listenSocket)
        {
          us_socket_t * socket = reinterpret_cast<us_socket_t *>(listenSocket); // this cast is safe
          listenSuccessRef += us_socket_is_closed(0, socket) ? 0U : 1U;
        }

        startLatch.count_down();
      });

      if (!wsApp.constructorFailed())
        wsApp.run();
    });
    

    if (thread && setThreadAffinity(thread->native_handle(), core))
      threads.push_back(thread);
    else
      std::cout << "Failed to assign io thread to core " << core << '\n';    
  }

  startLatch.wait();

  if (listenSuccess != nIoThreads)
    std::cout << "Failed to listen on " << ip << ":"  << port << std::endl;
  
  
  std::cout << "Ready\n";
  for(auto thread : threads)
    thread->join();

  
  if (kv::serverStats)
    delete kv::serverStats;
}

