#include <iostream>
#include <thread>
#include <algorithm>
#include <mutex>
#include <uwebsockets/App.h>
#include <core/FusionCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvHandler.h>


std::mutex stdoutMutex;

using namespace fusion::core;
namespace kv = fusion::core::kv;


int main()
{
  std::cout << "Fusion v" << FUSION_VERSION << " starting\n";

  std::size_t nPools = std::max<std::size_t>(1U, std::thread::hardware_concurrency()); // TODO
  std::size_t nIoThreads = 4U;    // TODO

  kv::KvHandler handlers{nPools - nIoThreads, nIoThreads}; 
  std::vector<std::jthread *> threads;

  for (std::size_t i = 0U, core = 0 ; i < nIoThreads ; ++i, ++core)
  {
    auto * thread = new std::jthread([&handlers]()
    {
      uWS::App().ws<kv::KvRequest>("/*",
      {
        // Settings
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = 16 * 1024,
        .idleTimeout = 120,
        .maxBackpressure =  1024 * 1024,
        // Handlers
        .upgrade = nullptr,
        .open = [](kv::KvWebSocket * ws)
        {
          ws->getUserData()->ws = ws;
        },
        .message = [&handlers](kv::KvWebSocket * ws, std::string_view message, uWS::OpCode opCode)
        {   
          //ws->send(message, opCode);
          //ws->getUserData()->ws->send(message, opCode);
          //ws->cork([ws, message, opCode](){ ws->send(message, opCode); });
    
          if (auto request = kv::kvjson::parse(message, nullptr, false); request.is_discarded())
          {
            ws->send("Invalid JSON", uWS::OpCode::TEXT);
          }
          else
          {
            ws->getUserData()->json = std::move(request);
            
            if (!handlers.handle(ws->getUserData()))
            {
              ws->send("handler call failed", uWS::OpCode::TEXT);
            }
          }
        },
        .drain = [](auto */*ws*/)
        {
          
        },
        .ping = [](auto */*ws*/, std::string_view)
        {

        },
        .pong = [](auto */*ws*/, std::string_view)
        {

        },
        .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/)
        {
          std::cout << "in close\n";
        }
      })
      .listen(1987, [](auto *listen_socket)
      {
        if (listen_socket)
        {
          std::cout << "Thread " << std::this_thread::get_id() << " listening on port " << 1987 << std::endl;
        }
        else
        {
          std::cout << "Thread " << std::this_thread::get_id() << " failed to listen on port 1987" << std::endl;
        }
      })
      .run();
    });

    if (!setThreadAffinity(thread->native_handle(), core))
      std::cout << "Failed to assign io thread to core " << core << '\n';

    threads.push_back(thread);
  }

  for(auto thread : threads)
    thread->join();
}

