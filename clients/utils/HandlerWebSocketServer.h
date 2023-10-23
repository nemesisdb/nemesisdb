#ifndef _FC_TEST_HANDLERWEBSOCKETSERVER_H
#define _FC_TEST_HANDLERWEBSOCKETSERVER_H


#include <map>
#include <vector>
#include <boost/asio.hpp>
#include "WebSocketServer.h"


namespace fusion { namespace test {

  class HandlerWebSocketServer
  {
  public:
    using SessionHandler = std::function<void(std::string)>;


    struct Handler
    {
      Handler(std::shared_ptr<SessionHandler> handler) : m_handler(handler)
      {

      }


      inline auto operator()(std::shared_ptr<WebSocketStream> ws, std::string urlPath) -> asio::awaitable<void>      
      {
        try
        {
          bool connected = true;
          std::string body;
          body.reserve(ws->read_message_max());

          while (connected)
          {
            beast::error_code ec;

            auto buffer = asio::dynamic_buffer(body);

            const auto bytesRead = co_await ws->async_read(buffer, asio::redirect_error(asio::use_awaitable, ec));
            connected = ec.value() == 0;

            if (m_handler && ws->got_text() && !ws->got_binary())
            {
              (*m_handler)(std::move(body));
              body.clear();
            }
          }
        }
        catch(...)
        {

        }

        co_return;
      }

      std::shared_ptr<SessionHandler> m_handler;
    };


    HandlerWebSocketServer(std::shared_ptr<asio::io_context> ioc, std::shared_ptr<SessionHandler> onSession) : m_server(ioc), m_onSession(onSession)
    {     
    }

    
    void start (const std::string host, const Port port, const std::size_t maxRead)
    {     
      m_server.start(host, port, maxRead, m_onSession);
    }
    

    void stop()
    {
      m_server.stop();
    }


  private:
    WebSocketServerImpl<Handler, std::shared_ptr<SessionHandler>> m_server;
    std::shared_ptr<SessionHandler> m_onSession; 
  };


  } // ns core
} // ns fusion

#endif
