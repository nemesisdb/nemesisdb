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

const std::size_t MaxSessions = 1'000'000;

std::latch signalLatch{1};
std::vector<nemesis::core::SessionToken> tokens;
std::size_t nSessions = 1;
std::size_t nIo = std::thread::hardware_concurrency();
std::string serverIp = "127.0.0.1";
int serverPort = 1987;
bool createLog = false;
bool setQ = true;

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


const auto jsonSet = R"({
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


bool readProgramArgs(const int argc, char ** argv)
{
  po::options_description common("");
  common.add_options()    
    ("io",        po::value<std::size_t>(&nIo),         "Number of IO threads")
    ("sessions",  po::value<std::size_t>(&nSessions),   "Number of sessions")
    ("ip",        po::value<std::string>(&serverIp),    "Server IP")
    ("port",      po::value<int>(&serverPort),          "Server port")
    ("log",       "Log session tokens to session.txt")
    ("set",       "Use KV_SET instead of KV_SETQ (default: false)");


  po::options_description all;
  all.add(common);

  try
  {
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, all), vm);

    po::notify(vm);

    createLog = vm.contains("log");
    setQ = !vm.contains("set");
  }
  catch (const po::unknown_option& uo)
  {
    std::cout << "ERROR: unknown option: " << uo.get_option_name() << '\n';
    return false;
  }

  return true;
}


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
          tokens.emplace_back(rsp["SH_NEW_RSP"]["tkn"].get<SessionToken>());
        }
      }

      latch.count_down();
    }
  };

  if (auto ws = client.openQueryWebSocket(ip, port, "", onRsp); ws)
  {
    auto start = Clock::now();

    for (auto i = 0 ; i < nSessions; ++i)
      ws->send(R"({"SH_NEW":{ "name":")"+std::to_string(i)+"\" }}");

    latch.wait();

    auto end = Clock::now();
    std::cout << "New:\t" << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << '\n';
  }
  else
    std::cout << "createSessions(): falied to connect\n";    
}


void set(const std::string& ip, const int port, std::shared_ptr<asio::io_context> ioc, const bool setq)
{
  std::latch latch {static_cast<std::ptrdiff_t>(tokens.size())};

  auto setRsp = [&latch](Response r)
  {
    if (r.connected)
    {
      latch.count_down();
    }
  };

  fusion::client::ResponseHandler handler = [](Response r){}; // if setq, no response unless error, we assume no errors here

  if (!setq)
    handler = setRsp;

  fusion::client::Client client{*ioc};

  if (auto ws = client.openQueryWebSocket(ip, port, "", handler); ws)
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
    std::cout << (setq ? "SetQ" : "Set") << ":\t" << duration_cast<milliseconds>(end-start).count() << '\n';
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
      // auto rsp = json::parse(response.msg);
      // if (rsp.contains("KV_GET_RSP"))
      // {
      //   auto& keys = rsp["KV_GET_RSP"]["keys"];
      //   if (keys.contains("profile") && keys.contains("interests"))
      //   {
      //     if (keys["profile"].size() != 3 || keys["interests"].size() != 4 || keys["interests"].size() != 5)
      //       fails += 1;
      //   }
      // }

      latch.count_down();
    }
  };


  if (auto ws = client.openQueryWebSocket(ip, port, "", onRsp); ws)
  {
    json query;
    //query["KV_GET"]["tkn"] = json{};
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
    std::cout << "Get:\t" << duration_cast<milliseconds>(end-start).count() << '\n';
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
        if (rsp["SH_NEW_RSP"]["st"] == (std::size_t)nemesis::core::RequestStatus::Ok)
        {
          std::scoped_lock lck {mux};
          tokens.emplace_back(rsp["SH_NEW_RSP"]["tkn"].get<SessionToken>());
        }
      }

      latch.count_down();
    }
  };


  if (auto ws = client.openQueryWebSocket(ip, port, "", onRsp); ws)
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

    std::cout << "Get:\t" << duration_cast<milliseconds>(end-start).count() << '\n';
  }
  else
    std::cout << "createSessions(): falied to connect\n";
}


int main (int argc, char ** argv)
{
  if (readProgramArgs(argc, argv))
  {
    Ioc ioc{std::min<std::size_t>(nIo, std::thread::hardware_concurrency())};

    if (createSessions(serverIp, serverPort, ioc.ioc, std::min<std::size_t>(MaxSessions, nSessions)); !tokens.empty())
    {
      if (createLog)
      {
        std::ofstream out{"sessions.txt", std::ios_base::trunc | std::ios_base::out};
        std::for_each(tokens.cbegin(), tokens.cend(), [&out](const auto& tkn){ out << tkn << ",\n";});
      }

      set(serverIp, serverPort, ioc.ioc, setQ);
      get(serverIp, serverPort, ioc.ioc);
    }
  }

  return 0;
}
