#include <iostream>
#include <thread>
#include <algorithm>
#include <mutex>
#include <uwebsockets/App.h>
#include <core/FusionCommon.h>


std::mutex stdoutMutex;

using namespace fusion::core;

int main()
{
  std::cout << "Fusion v" << FUSION_VERSION << " starting\n";

  std::size_t nIoThreads = 4U;
  
  std::vector<std::jthread *> threads;

  for (std::size_t i = 0U, core = 0 ; i < nIoThreads ; ++i, ++core)
  {
    auto * thread = new std::jthread([]()
    {
      uWS::App().ws<QueryData>("/*",
      {
        // Settings
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = 16 * 1024,
        .idleTimeout = 10,
        .maxBackpressure = 1 * 1024 * 1024,
        // Handlers
        .upgrade = nullptr,
        .open = [](WebSocket * ws)
        {
          ws->getUserData()->ws = ws;
        },
        .message = [](WebSocket * ws, std::string_view message, uWS::OpCode opCode)
        {   
          ws->send(message, opCode);
          ws->getUserData()->ws->send(message, opCode);
          //ws->cork([ws, message, opCode](){ ws->send(message, opCode); });
        },
        .drain = [](auto */*ws*/)
        {
          /* Check getBufferedAmount here */
        },
        .ping = [](auto */*ws*/, std::string_view)
        {

        },
        .pong = [](auto */*ws*/, std::string_view)
        {

        },
        .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/)
        {

        }
      })
      .listen(9001, [](auto *listen_socket)
      {
        if (listen_socket)
        {
          std::cout << "Thread " << std::this_thread::get_id() << " listening on port " << 9001 << std::endl;
        }
        else
        {
          std::cout << "Thread " << std::this_thread::get_id() << " failed to listen on port 9001" << std::endl;
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

