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
std::size_t valueSize = 0;


struct Ioc
{
  Ioc(const std::size_t core) : Ioc()
  {
    nemesis::core::setThreadAffinity(thread.native_handle(), core);
  }

  Ioc() : ioc(std::make_shared<asio::io_context>(1))
  {
    thread = std::move(std::jthread{[this]()
    {
      auto workGuard = asio::make_work_guard(*ioc);
      ioc->run();
      stoppedCv.notify_one();
    }});
  }

  ~Ioc()
  {
    if (ioc && !ioc->stopped())
    {
      ioc->stop();

      std::unique_lock lck {stoppedMux};
      stoppedCv.wait(lck, [this](){ return ioc->stopped(); });
    }
  }

  
  std::shared_ptr<asio::io_context> ioc;
  
  private:
    std::jthread thread;
    std::condition_variable stoppedCv;
    std::mutex stoppedMux;
};


void sigHandle(int param)
{
  latch.count_down();
}


inline UUIDv4::UUID createUuid ()
{
  static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator; 
  return uuidGenerator.getUUID();
}





bool parseConfig (const int argc, char ** argv)
{
  po::options_description common("");
  common.add_options()
    ("help",   "Show help")
    ("config",    po::value<std::filesystem::path>(&configPath),  "Path to JSON config")
    ("receiveIp", po::value<std::string>(&receiveIp),             "IP to listen for config");
      
  po::options_description all;
  all.add(common);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, all), vm);
  po::notify(vm);

  bool valid = false;

  if (vm.contains("help"))
    std::cout << all << '\n';
  else
  {
    if (vm.contains("receiveIp"))
    {
      mode = Mode::Receive;
      valid = true;
    }
    else if (!std::filesystem::exists(configPath))
      std::cout << "Config path not found\n";
    else
    {
      auto isValid = [](std::function<bool()> isValidCheck, const std::string_view msg)
      {
        const auto valid = isValidCheck();
        if (!valid)
          std::cout << "\n** Config Error **\n" << msg << "\n****\n";
        return valid;
      };

      std::ifstream cfgStream {configPath};
      config = json::parse(cfgStream);

      if (config.size() != 1U)
        std::cout << "Root must have only one key: Local or SendConfig\n";
      else
      {
        auto validateLocal = [&isValid](json& config)
        {
          bool valid =  isValid( [&]{ return config.contains("fusionIp") && config.contains("fusionPort") && config.contains("nClients") && config.contains("valueSize") && config.contains("set") && config.contains("get") ; },
                                 "Config must contain ip, port, nClients and either datafiles or set and get") &&
                        isValid( [&]{ return config["fusionIp"].is_string() && config["fusionPort"].is_number_unsigned() && config["nClients"].is_number_unsigned() && config["valueSize"].is_number_unsigned(); },
                                 "ip must be string, port, nClients and valueSize must be unsigned int") &&
                        isValid([&]{ valueSize = config["valueSize"] ; return valueSize == 50 || valueSize == 100 || valueSize == 1000; },
                                 "valueSize can onle be either: 50, 100 or 1000");;

          if (valid)
          {
            valid = isValid([&]{ return config["set"].is_object() && config["set"].contains("max") && config["set"]["max"].is_number_unsigned() && config["set"]["max"] > 0U; }, "set must be object with max as unsigned int > 0") &&
                    isValid([&]{ return config["set"].contains("setq") && config["set"]["setq"].is_boolean(); }, "set must contain a boolean setq flag") &&
                    isValid([&]{ return config["get"].is_object() && config["get"].contains("enabled") && config["get"]["enabled"].is_boolean(); }, "get must object with 'enabled' boolean");
          }
          
          return valid;
        };

        if (config.begin().key() == "Local")
        {
          valid = validateLocal(config["Local"]);
          mode = Mode::Local;
        }
        else if (config.begin().key() == "Send")
        {
          mode = Mode::Send;

          auto& sendCfg = config["Send"];
          valid = isValid([&]{return sendCfg.contains("clients") && sendCfg["clients"].is_array();},  "SendConfig must contain 'clients' with IP addresses") &&
                  isValid([&]{return sendCfg.contains("config") && sendCfg["config"].is_object();},   "SendConfig must contain 'config' object");
        }
      }
    }
  }

  return valid;
}



struct TestClient
{
  struct Result
  {
    std::chrono::microseconds setDuration;
    std::chrono::microseconds getDuration;
  };


  TestClient(json& conf) : config(conf)
  {
  }



  void createData(const std::size_t maxKeys) 
  {
    auto valueIndex = 0;
    
    if (valueSize == 100)
      valueIndex = 1;
    else if (valueSize == 1000)
      valueIndex = 2;


    auto createKey = []() -> std::string
    {
      return createUuid().str();
    };
    
    auto createValue = [valueIndex]() -> json
    {
      static std::random_device dev;
      static std::mt19937 rng(dev());

      json value;

      static const std::array<const std::string_view, 3U> strings =
      {
        "asdasdfdfdoisfgsia20asdasdfdfdoisfgsia20asdasdfdfd",
        "asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20",
        "asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20"
        "asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20"
        "asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20"
        "asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20"
        "asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20asdasdfdfdoisfgsia40asdasdfdfdoisfgsia20"
      };
      
      return strings[valueIndex];
    };

    try
    {
      for (std::size_t i = 0; i < maxKeys ; ++i)
        data.emplace(createKey(), createValue());
    }
    catch (std::exception& ex)
    {
      std::cout << ex.what() << '\n';
      data.clear();
    }
  }


  bool createSession ()
  {
    std::latch latch{1};
    json rsp;

    auto onResponse = [&latch, &rsp](fusion::client::Response r)
    {
      if (r.connected)
      {
        rsp = json::parse(std::move(r.msg));
        latch.count_down();
      }
    };
    

    fusion::client::Client client {*ioc.ioc};
    if (auto ws = client.openQueryWebSocket(config["fusionIp"], config["fusionPort"], "/", onResponse); ws)
    {
      json sesh{{"SH_NEW", {{"name","test"}}}};
      ws->send(sesh.dump());

      latch.wait();

      if (rsp.contains("SH_NEW_RSP") && rsp["SH_NEW_RSP"]["st"] == 1)
      {
        token = rsp["SH_NEW_RSP"]["tkn"];
        return true;
      }
      else
      {
        std::cout << "Failed to create session:\n" << rsp << '\n';
      }
    }
  
    return false;
  }

  
  std::future<std::chrono::microseconds> set ()
  {
    auto future = std::async(std::launch::async, [this]()
    {
      const bool setq = config["set"]["setq"];
      std::latch done {static_cast<std::ptrdiff_t>(data.size())};
      
      auto onResponse = [setq, &done](fusion::client::Response rsp)
      {
        if (rsp.connected)
        {
          if (!setq)
            done.count_down();
        }
      };
      
      fusion::client::Client client {*ioc.ioc};
      if (auto ws = client.openQueryWebSocket(config["fusionIp"], config["fusionPort"], "/", onResponse); ws)
      {
        const auto queryName = setq ? "KV_SETQ" : "KV_SET";
        std::size_t nSent{0};

        NemesisTimePoint start = NemesisClock::now();

        for (auto& kv : data)
        {
          json query;
          query[queryName]["tkn"] = token;
          query[queryName]["keys"] = {{kv.first, kv.second}};

          //std::cout << query.dump() << '\n';

          ws->send(query.dump());
          ++nSent;
        }
        
        if (!setq)
          done.wait();

        return std::chrono::duration_cast<std::chrono::microseconds>(NemesisClock::now()-start);
      }
      else
        std::cout << "Could not connect to " << config["fusionIp"] << ":" << config["fusionPort"] << '\n';

      return std::chrono::microseconds{};
    });

    return future;
  }


  std::future<std::chrono::microseconds> get ()
  {
    auto future = std::async(std::launch::async, [this]()
    {
      const std::size_t max = data.size() ;
      std::latch done {static_cast<std::ptrdiff_t>(max)};

      auto onResponse = [&done](fusion::client::Response rsp)
      {
        if (rsp.connected)
          done.count_down();
      };
      
      fusion::client::Client client {*ioc.ioc};
      if (auto ws = client.openQueryWebSocket(config["fusionIp"], config["fusionPort"], "/", onResponse); ws)
      {
        auto start = NemesisClock::now();

        std::size_t i = 0;    
        for (auto& kv : data)
        {
          if (i++ == max)
            break;
          
          json query;
          query["KV_GET"]["tkn"] = token;
          query["KV_GET"]["keys"] = {std::move(kv.first)};

          //std::cout << query.dump() << '\n';

          ws->send(query.dump());
        }

        done.wait();
        return std::chrono::duration_cast<std::chrono::microseconds>(NemesisClock::now()-start);
      }
      else
        return std::chrono::microseconds{};
    });

    return future;
  }



private:
  Ioc ioc;
  std::map<std::string, json> data;
  json config;
  std::chrono::microseconds setDuration, getDuration;
  SessionToken token;
};


void runLocal(json& config)
{
  const auto doingGet = config.contains("get") && config["get"]["enabled"];
  const std::size_t nClients = config["nClients"];
  const bool setq = config["set"]["setq"];

  std::vector<TestClient *> clients;
  

  for (std::size_t i = 0; i < nClients ; ++i)
  {
    if (auto * client = new TestClient{config}; client->createSession())
    {
      clients.push_back(client);
      clients.back()->createData(config["set"]["max"]);
    }
  }
  
  std::vector<std::future<std::chrono::microseconds>> futures;

  std::cout << (setq ? "SETQ" : "SET") << ":\n";
  
  for(auto& c : clients)
    futures.emplace_back(c->set());

  std::size_t i = 1;
  for (auto& f : futures)
  {
    auto result = f.get();
    std::cout << "\tClient " << i++ << " Duration: " << result << '\n'; 
  }

  futures.clear();
   
  if (doingGet)
  {
    std::cout << "GET:\n";
    
    for(auto& c : clients)
      futures.emplace_back(c->get());

    i = 1;
    for (auto& f : futures)
    {
      auto result = f.get();
      std::cout << "\tClient " << i++ << " Duration: " << result << '\n'; 
    }
  }
}

  
int main (int argc, char ** argv)
{
  #ifdef NDEBUG
  if (!parseConfig(argc, argv))
    return 0;
  #else
    mode = Mode::Local;
    config["Local"]["fusionIp"] = "127.0.0.1";
    config["Local"]["fusionPort"] = 1987;
    config["Local"]["nClients"] = 1;
    config["Local"]["valueSize"] = 50;
    config["Local"]["set"]["max"] = 5;
    config["Local"]["set"]["setq"] = false;
    config["Local"]["get"]["enabled"] = true; 
  #endif

  const std::size_t nIoc = std::thread::hardware_concurrency();

  if (mode == Mode::Local)
  {
    runLocal(config["Local"]);
  }
  else if (mode == Mode::Receive)
  {
    std::latch haveConfig{1};

    json config;
    auto onSession = [&config, &haveConfig](std::string msg)
    {
      config = json::parse(msg);
      haveConfig.count_down();
    };

    auto ioc = std::make_shared<asio::io_context>(1U);
    std::jthread ioThread
    {
      [ioc]()
      {
        auto workGuard = asio::make_work_guard(*ioc);
        ioc->run();
      }
    };
    while (ioc->stopped());

    nemesis::test::HandlerWebSocketServer ws {ioc, std::make_shared<nemesis::test::HandlerWebSocketServer::SessionHandler>(onSession)};
    
    ws.start(receiveIp, 1987, 1'000'000U);

    haveConfig.wait();

    std::cout << "Have config: " << config << '\n';
    runLocal(config);

    ioc->stop();
  }
  else if (mode == Mode::Send)
  {
    auto ioc = std::make_shared<asio::io_context>(1U);
    std::jthread ioThread
    {
      [ioc]()
      {
        auto workGuard = asio::make_work_guard(*ioc);
        ioc->run();
      }
    };
    while (ioc->stopped());

    std::vector<fusion::client::WebSocketSession> sessions;

    fusion::client::Client client{*ioc};
    for (auto& ip : config["Send"]["clients"])
    {
      std::cout << ip << '\n';
      if (auto wsc = client.openQueryWebSocket(ip.get<std::string>(), 1987, "", [](auto rsp){}); wsc)
        sessions.emplace_back(std::move(wsc));
      else
      {
        std::cout << "Failed to connect to " << ip.get<std::string>() << ":" << 1987 << '\n';
        return 0;
      }
    }
    
    for (auto& session : sessions)
      session->send(config["Send"]["config"].dump());

    ioc->stop();
  }

  return 0;
}