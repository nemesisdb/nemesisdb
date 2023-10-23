#ifndef _FC_TEST_WEBSOCKETSERVER_H
#define _FC_TEST_WEBSOCKETSERVER_H


#include <thread>
#include <iostream>
#include <functional>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/spawn.hpp>
#include <nlohmann/json.hpp>
//#include <core/Common.h>


namespace nemesis { namespace test {

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace http = beast::http; 


using Port = boost::asio::ip::port_type;
using tcp = boost::asio::ip::tcp;
using WebSocketStream = beast::websocket::stream<beast::tcp_stream>;
using WebSocketSessionHandler = std::function<asio::awaitable<void>(std::shared_ptr<WebSocketStream>, std::string)>;
// TODO find out WTF this is - it was taken from a beast example
using AwaitableTcpStream = typename beast::tcp_stream::rebind_executor<asio::use_awaitable_t<>::executor_with_default<asio::any_io_executor>>::other;


template<class SessionHandler, typename ... Args>
class WebSocketServerImpl // TODO use concept to validate SessionHandler
{

private:  
  enum class State { NotStarted, Running, Stopping, Stopped };
  
public:

  WebSocketServerImpl(std::shared_ptr<asio::io_context> ioc) :
    m_ioc(ioc),
    m_state(State::NotStarted)
  {
  }


  ~WebSocketServerImpl()
  {
    stop();
  }


  void start(const std::string host, const Port port, const std::size_t maxRead, Args... args)
  {
    m_port = port;
    m_address = host;
    m_maxReadSize = maxRead;

    auto endpoint = tcp::endpoint{asio::ip::make_address(m_address), m_port};    
    
    asio::co_spawn(*m_ioc, std::bind(&WebSocketServerImpl<SessionHandler, Args...>::listen, std::ref(*this), std::move(endpoint), std::forward<Args>(args) ...), asio::detached);
  }


  void stop()
  {
    if (m_acceptor)
    {
      m_state = State::Stopping;

      try
      {
        m_acceptor->cancel();
        m_acceptor->close();
      }
      catch (...)
      {

      }    
    }

    m_state = State::Stopped;
  }


private:

    
  auto listen (tcp::endpoint endpoint, Args ... args) -> asio::awaitable<void>
  {
    auto executor = co_await asio::this_coro::executor;

    try
    {
      m_acceptor = std::make_shared<tcp::acceptor>(tcp::acceptor{executor, endpoint, true});
      m_acceptor->set_option(tcp::no_delay(true));
    }
    catch (const boost::system::system_error se)
    {
      std::cout << "Error starting TCP acceptor for WebSocket server on " << endpoint << "\nError: " << se.what() << '\n';
    }

    if (m_acceptor)
    {
      std::cout << "Started WebSocket server on " << endpoint << '\n';

      m_state = State::Running;
      
      try
      {
        while (m_state == State::Running)
        {
          auto socket = co_await m_acceptor->async_accept(asio::make_strand(*m_ioc), asio::use_awaitable);

          if (m_state == State::Running)
          {
            asio::co_spawn (executor, [this, sock = std::move(socket), ... handlerArgs = args /*std::forward<Args>(args)*/]() mutable -> asio::awaitable<void>
            { 
              // SessionHandler can have a "bool accept(std::string&)" which passes the path
              //  1) to get the path, we need to handle the upgrade ourselves (rather than with WebSocketStream{sock})
              //  2) hijack the upgrade here, grabbing the path, call SessionHandler::accept() if present, then upgrade to a WebSocket if accepted
              // This technique adapted from: https://github.com/boostorg/beast/issues/751
              AwaitableTcpStream stream {std::move(sock)};
                              
              std::string body;
              body.reserve(64U);
              
              auto buffer = asio::dynamic_buffer(body);
              http::request<http::string_body> req;
              
              if (const auto n = co_await http::async_read(stream, buffer, req); n && websocket::is_upgrade(req))
              {
                auto handler = std::make_shared<SessionHandler>(handlerArgs...);
                bool sessionAccept = true;

                // user can supply a handler with an accept(), based on the requested target path
                if constexpr (requires (const std::string& path) { {handler->accept(path)} -> std::convertible_to<bool>; })
                {
                  sessionAccept = handler->accept(req.target());
                }

                if (sessionAccept)
                {
                  auto ws = std::make_shared<WebSocketStream>(std::move(stream));

                  // upgrade to WebSocket
                  ws->accept(req);

                  ws->read_message_max(m_maxReadSize);
      
                  // permessage_deflate and auto_fragment settings taken from beast's "fast server" example
                  websocket::permessage_deflate pmd;
                  pmd.client_enable = true;
                  pmd.server_enable = true;
                  pmd.compLevel = 3;

                  ws->set_option(pmd);
                  ws->set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
                  ws->set_option(websocket::stream_base::decorator([](websocket::response_type& res)
                  {
                    res.set(http::field::server, std::string("FusionCache"));
                  }));

                  ws->auto_fragment(false);

                  co_await (*handler)(ws, req.target());
                }
                else
                  sendResponse(stream, http::status::not_found);
              }
              else
                sendResponse(stream, http::status::upgrade_required);

              co_return;
            },
            asio::detached);
          }
        }
      }
      catch (const boost::system::system_error se)
      {
        // swallow, server shutdown
      }
      catch (const std::exception ex)
      {
        std::cout << "WebSocketServer unexpected exception: " << ex.what() << '\n';
      }
      catch (...)
      {
        std::cout << "WebSocketServer unknown exception\n";
      }
    }

    m_state = State::Stopped;

    co_return;
  }

  
  void sendResponse(AwaitableTcpStream& stream, const http::status status, const bool closeSocket = false)
  { 
    http::response<http::string_body> res {std::piecewise_construct, std::make_tuple(""), std::make_tuple(status, 11U)};
    res.set(http::field::server, "FusionCache");
    res.set(http::field::content_type, "application/text");
    res.prepare_payload();
    res.keep_alive(false);

    beast::error_code ec;
    http::write(stream, res, ec);

    if (closeSocket)
      stream.socket().close();
  }


private:
  std::size_t m_maxReadSize;
  std::shared_ptr<asio::io_context> m_ioc;
  std::atomic<State> m_state;
  std::shared_ptr<tcp::acceptor> m_acceptor;
  Port m_port;
  std::string m_address; 
};



} // ns core
} // ns fusion


#endif
