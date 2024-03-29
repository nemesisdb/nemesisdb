#ifndef NDB_CORE_TSSERVER_H
#define NDB_CORE_TSSERVER_H

#include <core/NemesisCommon.h>
#include <core/ts/TsCommon.h>
#include <core/ts/TsHandler.h>


namespace nemesis { namespace core { namespace ts {


class TsServer 
{
public:
  
  TsServer () : m_run(false)
  {

  }


  ~TsServer()
  {
    try
    {
      stop();  
    }
    catch(const std::exception& e)
    {
      PLOGF << "Error during shutdown";
    }
  }


  void stop()
  {
    m_run = false;

    {
      std::scoped_lock lck{m_wsClientsMux};
      for (auto ws : m_wsClients)
        ws->end(1000); // calls AsyncSocket::shutdown()

      m_wsClients.clear();
    }

    {
      for(auto sock : m_sockets)
        us_listen_socket_close(0, sock);

      m_sockets.clear();
    }
  }


  bool run (const NemesisConfig& config)
  {
    m_run = true;
    //m_serverStats = std::make_shared<ServerStats>();
    m_handler = std::make_shared<TsHandler>(config.cfg);

    const auto [ip, port, maxPayload] = NemesisConfig::wsSettings(config.cfg);
    const std::size_t preferredCore = NemesisConfig::preferredCore(config.cfg); 
    std::size_t core = 0;
    
    if (const auto maxCores = std::thread::hardware_concurrency(); maxCores == 0)
      PLOGE << "Could not acquire available cores";
    else if (preferredCore > maxCores)
      PLOGE << "'core' value in config is above maximum available: " << maxCores;
    else
      core = preferredCore;


    startWsServer(ip, port, maxPayload,  core);


    #ifndef NDB_UNIT_TEST
    PLOGI << "Ready";
    #endif

    return true;
  }

private:

  bool startWsServer (const std::string& ip, const int port, const unsigned int maxPayload, const std::size_t core)
  {
    bool listening{false};
    std::latch startLatch (1);

    auto listen = [this, ip, port, &listening, &startLatch, maxPayload]()
    {
      char requestBuffer[NEMESIS_KV_MAXPAYLOAD] = {};
      
      auto wsApp = uWS::App().ws<WsSession>("/*",
      {
        .compression = uWS::DISABLED,
        .maxPayloadLength = maxPayload,
        .idleTimeout = 180, // TODO should be configurable?
        .maxBackpressure = 24 * 1024 * 1024,
        // handlers
        .upgrade = nullptr,
        .open = [this](TsWebSocket * ws)
        {
          m_wsClients.insert(ws);
        },          
        .message = [this,  &requestBuffer](TsWebSocket * ws, std::string_view message, uWS::OpCode opCode)
        { 
          //++m_serverStats->queryCount;  actually do this

          if (opCode != uWS::OpCode::TEXT)
            ws->send(createErrorResponse(TsRequestStatus::OpCodeInvalid).to_string(), WsSendOpCode);
          else
          {
            njson request = njson::null();

            try
            {
              request = njson::parse(message);
            }
            catch (...)
            {
              // request remains null
            }
            
            // can't reliably parse command if JSON is invalid so send a general "ERR" response
            if (request.is_null())
              ws->send(createErrorResponse(TsRequestStatus::JsonInvalid).to_string(), WsSendOpCode);
            else if (request.empty() || !request.is_object())
              ws->send(createErrorResponse(TsRequestStatus::CommandSyntax).to_string(), WsSendOpCode);
            else
            {
              const auto& commandName = request.object_range().cbegin()->key();

              PLOGD << commandName;

              if (const auto status = m_handler->handle(ws, commandName, std::move(request)); status != TsRequestStatus::Ok)
                ws->send(createErrorResponse(status).to_string(), WsSendOpCode);
            }
          }
        },
        .close = [this](TsWebSocket * ws, int /*code*/, std::string_view /*message*/)
        {
          ws->getUserData()->connected->store(false);

          // when we shutdown, we have to call ws->end() to close each client otherwise uWS loop doesn't return,
          // but when we call ws->end(), this lambda is called, so we need to avoid mutex deadlock with this flag
          if (m_run)
            m_wsClients.erase(ws);
        }
      })
      .listen(ip, port, [this, port, &listening, &startLatch](auto * listenSocket)
      {
        if (listenSocket)
        {
          m_sockets.push_back(listenSocket);

          us_socket_t * socket = reinterpret_cast<us_socket_t *>(listenSocket); // this cast is safe
          listening = us_socket_is_closed(0, socket) == 0U;
        }

        startLatch.count_down();
      });
      

      if (!wsApp.constructorFailed())
        wsApp.run();          
    };


    bool started = false;
    try
    {
      m_thread.reset(new std::jthread(listen));

      startLatch.wait();

      if (listening)
      {
        if (setThreadAffinity(m_thread->native_handle(), core))
        {
          PLOGI << "Server assigned to core " << core;
          started = true;
        }
        else
          PLOGE << "Failed to assign server to core " << core;
      }
      else
        PLOGE << "Failed to start WS server";
    }
    catch(const std::exception& e)
    {
      PLOGF << "Failed to start WS thread:\n" << e.what();
    }

    return started;
  }


private:
  std::vector<us_listen_socket_t *> m_sockets;
  std::set<TsWebSocket *> m_wsClients;
  std::unique_ptr<std::jthread> m_thread;
  std::mutex m_wsClientsMux;
  std::shared_ptr<TsHandler> m_handler; // TODO this is copy constructed on every call to uWs's 'message' lambda, check performance vs raw pointer
  //std::shared_ptr<ServerStats> m_serverStats;
  std::atomic_bool m_run;
  
};


} // ns nemesis
} // ns core
} // ns ts


#endif
