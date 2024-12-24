#ifndef NDB_CORE_NEMESISCONFIG_H
#define NDB_CORE_NEMESISCONFIG_H

#include <string_view>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <boost/program_options.hpp>
#include <core/NemesisCommon.h>


namespace nemesis { 

  struct InterfaceSettings
  {
    std::string ip;
    int port;
    std::size_t maxPayload;
  };

  struct ArraySettings
  {
    std::size_t maxCapacity{};
    std::size_t maxRspSize{};
  };

  struct ListSettings
  {
    std::size_t maxRspSize{};
  };

  struct Settings
  {
  private:
    Settings() : valid(false)
    {

    }
    
    Settings(const njson& cfg)
    {
      interface.ip = cfg.at("ip").as_string();
      interface.port = cfg.at("port").as<int>(),
      interface.maxPayload = cfg.at("maxPayload").as<std::size_t>();

      loadOnStartup = false;
      maxPayload = cfg.at("maxPayload").as<std::size_t>();
      preferredCore = cfg["core"].as<std::size_t>();
      persistEnabled = cfg.at("persist").at("enabled") == true;
      persistPath = cfg.at("persist").at("path").as_string();

      arrays.maxCapacity = cfg.at("arrays").at("maxCapacity").as<std::size_t>();
      arrays.maxRspSize = cfg.at("arrays").at("maxResponseSize").as<std::size_t>();

      lists.maxRspSize = cfg.at("lists").at("maxResponseSize").as<std::size_t>();

      valid = true;
    }

    Settings(const njson& cfg, const fs::path startupLoadPath, const std::string_view startupLoadName) : Settings(cfg)
    {
      this->startupLoadName = startupLoadName;
      this->startupLoadPath = startupLoadPath;
      loadOnStartup = true;
      valid = true;
    }


  public:
    static void init(const njson& cfg)
    {
      settings = Settings{cfg};
    }

    static void init(const njson& cfg, const fs::path startupLoadPath, const std::string_view startupLoadName)
    {
      settings = Settings{cfg, startupLoadPath, startupLoadName} ;
    }

    static void init()
    {
      // create invalid settings
      settings = Settings{};
    }

    static const Settings& get()
    {
      return settings;
    }


    std::string wsSettingsString () const
    {
      return interface.ip + ":" + std::to_string(interface.port);
    }


    bool valid{false};
    InterfaceSettings interface;
    ArraySettings arrays;
    ListSettings lists;
    std::string startupLoadName;
    fs::path startupLoadPath;
    std::size_t maxPayload;
    std::size_t preferredCore;
    bool loadOnStartup;
    bool persistEnabled;
    fs::path persistPath;

  
  private:
    static Settings settings;
  };

  
  inline bool isValid (std::function<bool()> isValidCheck, const std::string_view msg)
  {
    const auto valid = isValidCheck();
    if (!valid)
    {
      PLOGF << "\n** Config Error **\n" << msg << "\n****\n";
    }
      
    return valid;
  };


  bool validatePersist (const njson& saveCfg)
  {
    return  isValid([&saveCfg]{ return saveCfg.contains("path") && saveCfg.at("path").is_string(); }, "persist::path must be a string") &&
            isValid([&saveCfg]{ return saveCfg.contains("enabled") && saveCfg.at("enabled").is_bool(); }, "persist::enabled must be a bool") && 
            isValid([&saveCfg]{ return !saveCfg.at("enabled").as_bool() || (saveCfg.at("enabled").as_bool() && !saveCfg.at("path").as_string().empty()); }, "persist enabled but path is empty");
  }


  bool validateArrays(const njson& arrays)
  {
    return  isValid([&arrays]{ return arrays.contains("maxCapacity") && arrays.at("maxCapacity").is_uint64(); }, "arrays::maxCapacity must be an integer") &&
            isValid([&arrays]{ return arrays.contains("maxResponseSize") && arrays.at("maxResponseSize").is_uint64(); }, "arrays::maxResponseSize must be an integer");
  }

  bool validateLists(const njson& lists)
  {
    return  isValid([&lists]{ return lists.contains("maxResponseSize") && lists.at("maxResponseSize").is_uint64(); }, "lists::maxResponseSize must be an integer");
  }


  std::tuple<bool, njson> parse(const fs::path& path)
  {
    try
    {
      std::ifstream configStream{path};
      const njson cfg = njson::parse(configStream);

      // TODO this is becoming untidy

      bool valid =  isValid([&cfg]{ return cfg.contains("persist") && cfg.at("persist").is_object(); },    "Require persist section") &&
                    isValid([&cfg]{ return cfg.contains("arrays") && cfg.at("arrays").is_object(); },      "Require arrays section") &&
                    isValid([&cfg]{ return cfg.contains("lists") && cfg.at("lists").is_object(); },        "Require lists section") &&
                    isValid([&cfg]{ return cfg.contains("version") && cfg.at("version").is_uint64(); },    "Require version as an unsigned int") &&
                    isValid([&cfg]{ return cfg.at("version") == nemesis::NEMESIS_CONFIG_VERSION; },        "Config version must be " + std::to_string(nemesis::NEMESIS_CONFIG_VERSION)) &&
                    isValid([&cfg]{ return !cfg.contains("core") || (cfg.contains("core") && cfg.at("core").is_uint64()); },  "'core' must be an unsigned integer") &&
                    isValid([&cfg]{ return cfg.contains("ip") && cfg.contains("port") && cfg.contains("maxPayload"); },       "Require ip, port, maxPayload and save") &&
                    isValid([&cfg]{ return cfg.at("ip").is_string() && cfg.at("port").is_uint64() && cfg.at("maxPayload").is_uint64(); }, "ip must string, port and maxPayload must be unsigned integer") &&
                    isValid([&cfg]{ return cfg.at("maxPayload") >= nemesis::NEMESIS_MINPAYLOAD; }, "maxPayload below minimum") &&
                    isValid([&cfg]{ return cfg.at("maxPayload") <= nemesis::NEMESIS_MAXPAYLOAD; }, "maxPayload exceeds maximum") ;

      
      if (valid &&
          validatePersist(cfg.at("persist")) &&
          validateArrays(cfg.at("arrays")) && 
          validateLists(cfg.at("lists")))
      {
        return {true, cfg};
      }
    }
    catch(const std::exception& e)
    {
      PLOGF << "Config file not valid JSON";
    }

    return {false, njson{}};
  }


  void readConfig (const int argc, char ** argv)
  {
    namespace po = boost::program_options;

    std::filesystem::path cfgPath;
    std::string loadName, cmdLoadPath;

    po::options_description common("");
    common.add_options()("help, h",    "Show help");

    po::options_description configFile("Config file");
    configFile.add_options()("config",  po::value<std::filesystem::path>(&cfgPath), "Path to json config file");

    po::options_description load("Load data");
    load.add_options()("loadPath",  po::value<std::string>(&cmdLoadPath), "Path to where to find loadName. If not set, will use persist::path from config");
    load.add_options()("loadName",  po::value<std::string>(&loadName), "Name of the save point to load");
    
    
    po::options_description all;
    all.add(common);
    all.add(configFile);
    all.add(load);
    
    po::variables_map vm;
    bool parsedArgs = false;

    try
    {
      po::store(po::parse_command_line(argc, argv, all), vm);
      po::notify(vm);
      parsedArgs = true;
    }
    catch(const po::error_with_option_name& pex)
    {
      PLOGF << pex.what();
    }
    catch(...)
    {
      PLOGF << "Unknown error reading program options";
    }


    Settings::init(); // initialise as invalid    

    if (parsedArgs)
    {
      if (vm.contains("help"))
        std::cout << all << '\n'; // intentional cout
      else
      { 
        if (vm.count("config") != 1U)
          std::cout << "Must set one --config option, with path to the JSON config file\n";
        else
        {
          if (!std::filesystem::exists(cfgPath))
          {
            PLOGF << "Config file path not found";
          }
          else if (const auto& [valid, cfg] = parse(cfgPath); valid)
          {
            if (vm.count("loadPath") || vm.count("loadName"))
            {            
              const fs::path loadPath = vm.count("loadPath") ? cmdLoadPath : cfg["persist"]["path"].as_string();

              if (!std::filesystem::exists(loadPath))
              {
                PLOGF << "Load path does not exist: " << loadPath ;
              }
              else
                Settings::init(cfg, loadPath, loadName);
            }
            else
              Settings::init(cfg);
          } 
        }
      }
    }
  }


} // namespace nemesis

#endif

