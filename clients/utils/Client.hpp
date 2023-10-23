#ifndef _FC_TEST_WSCLIENT_H
#define _FC_TEST_WSCLIENT_H


#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <string_view>
#include <chrono>
#include <functional>
#include <iostream>

// TODO can replace server with uWebSockets

namespace fusion { namespace client {


namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace http = beast::http;

using tcp = asio::ip::tcp;
using wsstream = websocket::stream<beast::tcp_stream>;

struct Response
{
  bool connected;
  std::string msg;
};

struct RestResponse
{
  bool success {true};
  std::string msg;
};


using ResponseHandler = std::function<void(Response rsp)>;


class WsSession
{
public:

  WsSession (asio::io_context& ioc, wsstream&& stream, ResponseHandler&& handler) :
    m_connected(true),
    m_stream(std::move(stream)),
    m_handler(std::move(handler)),
    m_clientDisconnected(false),
    m_ioc(ioc)
  {    
    
  }

  
  ~WsSession()
  {
    close(false);
  }


  void start()
  {
    asio::co_spawn(m_ioc, std::bind(&WsSession::read, std::ref(*this)), asio::detached);
  }

  
  void close (const bool clientDisconnect = true)
  {
    m_clientDisconnected = clientDisconnect;
    m_connected = false;

    if (clientDisconnect)
      m_handler = nullptr;

    if (m_stream.is_open())
    {
      beast::error_code ec;
      m_stream.close(websocket::close_code::normal, ec);
    }
  }


  inline void send (const std::string_view query)
  {
    if (m_stream.is_open()/*!m_clientDisconnected*/)
      m_stream.write(asio::buffer(query, query.size()));
  }


  // This is not for binary data, but for a query that is in memory after being read from a file.
  inline void send(const std::vector<char>& queryBuffer)
  {
    if (m_stream.is_open()/*!m_clientDisconnected*/)
      m_stream.write(asio::buffer(queryBuffer.data(), queryBuffer.size()));
  }


private:

  inline asio::awaitable<void> read()
  {
    beast::error_code ec;
    std::string receiveBuffer;

    while (m_connected)
    {
      try
      {
        receiveBuffer.reserve(2056U);
        auto buffer = asio::dynamic_buffer(receiveBuffer);
        
        const auto nBytes = co_await m_stream.async_read(buffer, asio::redirect_error(asio::use_awaitable, ec));
        
        m_connected = ec.value() == 0;
        
        if (nBytes && m_connected && m_handler)
          m_handler(Response {.connected = true, .msg = std::move(receiveBuffer)} );
        
        receiveBuffer.clear(); // because we moved-from
      }
      catch (std::exception& ex)
      {
        m_connected = false;
        std::cout << "client died: " << ex.what() << '\n';
      }
      catch (...)
      {
        m_connected = false;
        std::cout << "client died, unknown why\n";
      }
    }
    
    // if client disconnects, their handler may be destroyed
    if (m_clientDisconnected == false && m_handler)
      m_handler(Response {.connected = false});

    co_return;
  }


private:
  wsstream m_stream;  
  ResponseHandler m_handler;
  std::atomic_bool m_connected;
  std::atomic_bool m_clientDisconnected;
  asio::io_context& m_ioc;
};


using WebSocketSession = std::shared_ptr<WsSession>;


class Client
{
public:
  
  Client(asio::io_context& ioc, const std::string& userAgentName = "") : m_ioc(ioc), m_agentName(userAgentName)
  {

  }


  WebSocketSession openQueryWebSocket (std::string host, const short port, const std::string_view path, ResponseHandler onResponse)
  {
    WebSocketSession sesh;

    try    
    {
      wsstream ws {m_ioc};
      beast::get_lowest_layer(ws).expires_after(std::chrono::seconds(10));

      tcp::resolver resolver {m_ioc};
      auto resolveList = resolver.resolve(host, std::to_string(port));

      tcp::endpoint endPoint = beast::get_lowest_layer(ws).connect(resolveList);

      host += ':' + std::to_string(endPoint.port());

      // websockets have their own timeout rules
      beast::get_lowest_layer(ws).expires_never();
      // we're a client
      ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));
      // set user agent
      ws.set_option(websocket::stream_base::decorator([this](websocket::request_type& req)
      {
        req.set(http::field::user_agent, m_agentName);
        req.set(http::field::content_type, "application/json");
      }));
      ws.next_layer().socket().set_option(tcp::no_delay(true));

      ws.handshake(host, path.empty() ? "/" : path);

      // we're connected, run the session from a session object with a coroutine
      sesh = std::make_shared<WsSession>(m_ioc, std::move(ws), std::move(onResponse));
      sesh->start();
    }
    catch (...)
    {

    }
    
    return sesh;
  }


  RestResponse restQuery(const std::string_view host, const short port, const std::string_view query, const std::size_t readSize = 4096U)
  {
    using namespace std::literals;

    static const int HttpVersion = 11;
    static const auto RestPath = "/"sv;

    RestResponse response {.success = false};

    try
    {
      beast::error_code ec;

      tcp::resolver resolver {m_ioc};
      const auto resolved = resolver.resolve(std::string{host}, std::to_string(port));

      auto stream = beast::tcp_stream{m_ioc};      
      stream.expires_after(std::chrono::seconds(10));

      auto endpoint = stream.connect(resolved, ec);

      if (!ec)
      {
        http::request<http::string_body> req{http::verb::get, RestPath, HttpVersion};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, "FusionCache Client");
        req.set(beast::http::field::content_type, "application/json");
        req.body() = query;
        req.prepare_payload();

        http::write(stream, req, ec);

        if (!ec)
        {
          beast::flat_buffer buffer;
          buffer.reserve(readSize);
        
          http::response<http::string_body> rsp;  // may be better with dynamic_body if expecting larger payloads
          
          http::response_parser<http::string_body> parser{std::move(rsp)};
          parser.body_limit(readSize);
          
          http::read(stream, buffer, parser, ec);
          rsp = parser.release();

          if (!ec)
          {
            response.msg = std::move(rsp.body());
            response.success = true;
          }
          else
            response.msg = ec.what();
        }

        stream.close();
      }
    }
    catch (...)
    { 
      response.success = false;
    }

    return response;
  }


private:
  asio::io_context& m_ioc;
  std::string m_agentName;
};


} // ns client
} // ns fusion


#endif
