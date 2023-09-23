#include <iostream>
#include <thread>
#include <algorithm>
#include <mutex>
#include <uwebsockets/App.h>
#include <core/FusionCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvHandler.h>


using namespace fusion::core;
namespace kv = fusion::core::kv;


int main()
{
  std::cout << "Fusion v" << FUSION_VERSION << " starting\n";

  kv::serverStats = new kv::ServerStats;

  int port = 1987;
  
  const unsigned int maxPayloadDefault = 1024U;
  unsigned int maxPayloadMax = 2U * 1024U * 1024U;
  unsigned int maxPayload = maxPayloadDefault;  // TODO


  const auto nCores = std::min<std::size_t>(std::thread::hardware_concurrency(), FUSION_MAX_CORES);
  
  std::size_t nIoThreads = 1U;

  // TODO expirement
  if (nCores >= 48U)
    nIoThreads = 4U;
  else if (nCores >= 24U)
    nIoThreads = 3U;
  else if (nCores >= 16U)
    nIoThreads = 2U;
  
  kv::MaxPools = std::max<std::size_t>(1U, nCores - nIoThreads);

  kv::KvHandler handlers {kv::MaxPools, nIoThreads};
  std::vector<std::jthread *> threads;

  std::cout << "Total Cores: " << std::thread::hardware_concurrency() << '\n'
            << "Max Cores: " << nCores << '\n'
            << "I/O Threads: " << nIoThreads << '\n'
            << "Pools: " << kv::MaxPools << '\n'
            << "Pools First Core: " << nIoThreads << '\n'; 

  std::atomic_bool listenSuccess{true};

  for (std::size_t i = 0U, core = 0 ; i < nIoThreads ; ++i, ++core)
  {
    auto * thread = new std::jthread([&handlers, port, &listenSuccess, maxPayload, serverStats = kv::serverStats]()
    {
      auto wsApp = uWS::App().ws<kv::KvRequest>("/*",
      {
        .compression = uWS::DISABLED,
        .maxPayloadLength = maxPayload,
        .idleTimeout = 180,
        .maxBackpressure =  1024 * 1024,
        // handlers
        .upgrade = nullptr,
        .open = [](kv::KvWebSocket * ws)
        {
          ws->getUserData()->ws = ws; // ws pointer is valid until the client disconnects (after close lambda exits)
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
              ws->getUserData()->json = std::move(request);
              
              if (auto [status, msg] = handlers.handle(ws->getUserData()) ; status != kv::KvRequestStatus::Ok)
                ws->send(kv::createErrorResponse(status, msg).dump(), kv::WsSendOpCode);
            }
          }
        },
        .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/)
        {
          std::cout << "in close\n"; // TODO
        }
      })
      .listen(port, [port, &listenSuccess](auto * socket)
      {
        if (!socket)
          listenSuccess = false;          
      });

      if (!wsApp.constructorFailed())
        wsApp.run();
    });

    if (setThreadAffinity(thread->native_handle(), core))
      std::cout << "Assigned io thread to core " << core << '\n';
    else
      std::cout << "Failed to assign io thread to core " << core << '\n';

    threads.push_back(thread);
  }

  if (listenSuccess)
    std::cout << "Listening on port " << port << std::endl;
  else
    std::cout << "Failed to listen on port " << port << std::endl;

  for(auto thread : threads)
    thread->join();
}

