#include <latch>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/program_options.hpp>
#include <nlohmann/json.hpp>
#include <core/kv/KvCommon.h>
#include <core/NemesisCommon.h>
#include "utils/Client.hpp"
#include "utils/HandlerWebSocketServer.h"


using namespace fusion::client;
using namespace nemesis::core;
using namespace nemesis::core::kv;
using namespace std::chrono;

using json = nlohmann::json;
using Clock = std::chrono::steady_clock;


namespace po = boost::program_options;

enum class Mode
{
  None,
  Local,
  Send,
  Receive
};

std::latch latch{1};
std::string receiveIp;
std::filesystem::path configPath;
json config;
Mode mode = Mode::None;
std::vector<nemesis::core::SessionToken> tokens;


struct Ioc
{
  Ioc(const std::size_t nThreads)
  {
    ioc = std::make_shared<asio::io_context> (nThreads);

    for (std::size_t i = 0; i < nThreads ; ++i)
    {
      ioThreads.emplace_back([this]()
      {
        auto workGuard = asio::make_work_guard(*ioc);
        ioc->run();
        stoppedCv.notify_one();
      });

      nemesis::core::setThreadAffinity(ioThreads.back().native_handle(), i);
    }
  }

  ~Ioc()
  {
    if (ioc && !ioc->stopped())
    {
      ioc->stop();  // non-blocking call

      std::unique_lock lck {stoppedMux};
      stoppedCv.wait(lck, [this](){ return ioc->stopped(); });
    }
  }

  
  std::shared_ptr<asio::io_context> ioc;
  
  private:
    std::vector<std::jthread> ioThreads;
    std::condition_variable stoppedCv;
    std::mutex stoppedMux;
};


auto jsonSet = R"({
                    "KV_SET":
                    {
                      "tkn":"",
                      "keys":
                      {
                        "profile":
                        {
                          "handle":"some handle",
                          "email":"myemail@email.com",
                          "avatar":"abc.png"
                        },
                        "loginTime":0,
                        "interests":["Swimming", "Rugby", "Tennis", "Ice Fishing"],
                        "paymentMethods":[{"type":"Credit Card", "id":"1"}, {"type":"Credit Card", "id":"2"}],
                        "preferredPaymentMethod":"1"
                      }
                    }  
                  })"_json;


void createSessions(const std::string& ip, const int port, std::shared_ptr<asio::io_context> ioc, const std::size_t nSessions)
{
  std::mutex mux;
  std::latch latch{static_cast<std::ptrdiff_t>(nSessions)};


  fusion::client::Client client{*ioc};
  
  auto onRsp = [&mux, &latch](Response response)
  {
    if (response.connected)
    {      
      auto rsp = json::parse(response.msg);
      if (rsp.contains("SH_NEW_RSP"))
      {
        if (rsp["SH_NEW_RSP"]["st"] == 1)
        {
          std::scoped_lock lck {mux};
          tokens.emplace_back(rsp["SH_NEW_RSP"]["tkn"].get<std::string>());
        }
      }

      latch.count_down();
    }
  };


  if (auto ws = client.openQueryWebSocket(ip, 1987, "", onRsp); ws)
  {
    auto start = Clock::now();

    for (auto i = 0 ; i < nSessions; ++i)
      ws->send(R"({"SH_NEW":{ "name":")"+std::to_string(i)+"\" }}");

    latch.wait();

    auto end = Clock::now();
    std::cout << "New:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(end-start) << '\n';
  }
  else
    std::cout << "createSessions(): falied to connect\n";
}


void set(const std::string& ip, const int port, std::shared_ptr<asio::io_context> ioc, const bool setq)
{
  // TODO KV_SET
  fusion::client::Client client{*ioc};

  if (auto ws = client.openQueryWebSocket(ip, 1987, "", [](auto rsp){}); ws)
  {
    auto commandName = setq ? "KV_SETQ" : "KV_SET";

    std::size_t i = 0;
    auto start = Clock::now();

    for (const auto& tkn : tokens)
    {
      json command = setq ? json{{"KV_SETQ", jsonSet["KV_SET"]}} : jsonSet;

      command[commandName]["tkn"] = tkn;

      if (i % 3 == 0)
      {        
        command[commandName]["keys"]["paymentMethods"].emplace_back(json{{"type","BNPL"},{"id","3"}});
        command[commandName]["keys"]["interests"].emplace_back("Basketball");
      }

      ws->send(command.dump());

      ++i;
    }
    
    if (!setq)
      latch.wait();

    auto end = Clock::now();
    std::cout << (setq ? "SetQ" : "Set") << ":\t" << duration_cast<milliseconds>(end-start) << '\n';
  }
  else
    std::cout << "createSessions(): falied to connect\n";
}


void get(const std::string& ip, const int port, std::shared_ptr<asio::io_context> ioc)
{
  std::atomic_size_t fails{0};
  std::latch latch{static_cast<std::ptrdiff_t>(tokens.size())};

  fusion::client::Client client{*ioc};

  auto onRsp = [&latch, &fails](Response response)
  {
    if (response.connected)
    {      
      auto rsp = json::parse(response.msg);
      if (rsp.contains("KV_GET_RSP"))
      {
        auto& keys = rsp["KV_GET_RSP"]["keys"];
        if (keys.contains("profile") && keys.contains("interests"))
        {
          if (keys["profile"].size() != 3 || keys["interests"].size() != 4 || keys["interests"].size() != 5)
            fails += 1;
        }
      }

      latch.count_down();
    }
  };


  if (auto ws = client.openQueryWebSocket(ip, 1987, "", onRsp); ws)
  {
    json query;
    query["KV_GET"]["tkn"] ="";
    query["KV_GET"]["keys"] = json::array();

    auto start = Clock::now();

    for(const auto& tkn : tokens)
    {
      query["KV_GET"]["tkn"] = tkn;
      query["KV_GET"]["keys"] = json{"profile", "interests"};

      ws->send(query.dump());
    }
      
    latch.wait();

    auto end = Clock::now();
    std::cout << "Get:\t" << duration_cast<milliseconds>(end-start) << '\n';
  }
  else
    std::cout << "createSessions(): falied to connect\n";
}


void find(const std::string& ip, const int port, std::shared_ptr<asio::io_context> ioc)
{
  std::mutex mux;
  std::latch latch{static_cast<std::ptrdiff_t>(tokens.size())};

  fusion::client::Client client{*ioc};

  auto onRsp = [&mux, &latch](Response response)
  {
    if (response.connected)
    {      
      auto rsp = json::parse(response.msg);
      if (rsp.contains("SH_NEW_RSP"))
      {
        if (rsp["SH_NEW_RSP"]["st"] == 1)
        {
          std::scoped_lock lck {mux};
          tokens.emplace_back(rsp["SH_NEW_RSP"]["tkn"].get<std::string>());
        }
      }

      latch.count_down();
    }
  };


  if (auto ws = client.openQueryWebSocket(ip, 1987, "", onRsp); ws)
  {
    auto start = Clock::now();

    for(const auto& tkn : tokens)
    {
      json query;
      query["KV_GET"]["tkn"] = tkn;
      query["KV_GET"]["keys"] = json{"profile", "interests"};

      ws->send(query.dump());
    }
      
    latch.wait();

    auto end = Clock::now();

    std::cout << "Get:\t" << duration_cast<milliseconds>(end-start) << '\n';
  }
  else
    std::cout << "createSessions(): falied to connect\n";
}


int main (int argc, char ** argv)
{
  Ioc ioc{std::thread::hardware_concurrency()};
  
  if (createSessions("127.0.0.1", 1987, ioc.ioc, 10'000); !tokens.empty())
  {
    {
      std::ofstream out{"sessions.txt"/*std::to_string(Clock::now().time_since_epoch().count())+".txt"*/, std::ios_base::trunc | std::ios_base::out};
      std::for_each(tokens.cbegin(), tokens.cend(), [&out](const auto& tkn){ out << tkn << '\n';});
    }

    set("127.0.0.1", 1987, ioc.ioc, true);
    get("127.0.0.1", 1987, ioc.ioc);
  }

  return 0;
}