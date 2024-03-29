#ifndef NDB_CORE_NEMESISCONFIG_H
#define NDB_CORE_NEMESISCONFIG_H

#include <string_view>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <boost/program_options.hpp>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core {


struct NemesisConfig
{
  struct InterfaceSettings
  {
    std::string ip;
    int port;
    std::size_t maxPayload;
  };


  NemesisConfig() : valid(false)
  {

  }

  NemesisConfig(njson config) : cfg(std::move(config)), valid(true)
  {

  }

  NemesisConfig(const std::string_view config) : valid(true)
  {
    cfg = njson::parse(config);
  }

  
  static std::string kvSavePath (const njson& cfg)
  {
    return cfg.at("kv").at("sessions").at("save").at("path").as_string();
  }

  static bool kvSaveEnabled (const njson& cfg)
  {
    return cfg.at("kv").at("sessions").at("save").at("enabled").as_bool();
  }
  
  bool load() const
  {
    return !loadName.empty();
  }

  static bool haveSessions (const njson& cfg)
  {
    return cfg.at("kv").at("sessions").at("enabled") == true;
  }
  
  static std::size_t preferredCore(const njson& cfg) 
  {
    return cfg["core"].as<std::size_t>();
  }

  static std::size_t maxPayload(const njson& cfg) 
  {
    return cfg.at("maxPayload").as<std::size_t>();
  }

  static ServerMode serverMode (const njson& cfg)
  {
    return cfg.at("mode").as_string_view() == "kv" ? ServerMode::KV : ServerMode::TS;
  }

  static InterfaceSettings wsSettings (const njson& cfg)
  {
    return {.ip = cfg.at("ip").as_string(),
            .port = cfg.at("port").as<int>(),
            .maxPayload = cfg.at("maxPayload").as<std::size_t>() };
  }

  

  njson cfg;
  std::filesystem::path loadPath; // only used if started with --loadName
  std::string loadName; // set when started with --loadName
  bool valid;
};


inline bool isValid (std::function<bool()> isValidCheck, const std::string_view msg)
{
  const auto valid = isValidCheck();
  if (!valid)
    PLOGF << "\n** Config Error **\n" << msg << "\n****\n";
  return valid;
};


bool validateSave (const njson& saveCfg)
{
  return  isValid([&saveCfg]{ return saveCfg.contains("path") && saveCfg.at("path").is_string(); }, "save::path must be a string") &&
          isValid([&saveCfg]{ return saveCfg.contains("enabled") && saveCfg.at("enabled").is_bool(); }, "save::enabled must be a bool") && 
          isValid([&saveCfg]{ return !saveCfg.at("enabled").as_bool() || (saveCfg.at("enabled").as_bool() && !saveCfg.at("path").as_string().empty()); }, "save enabled but save::path is empty");
}


bool parseKv (njson& cfg)
{
  const auto& kvCfg = cfg.at("kv");

  bool valid =  isValid([&kvCfg]{ return kvCfg.is_object(); }, "kv must be an object") &&
                isValid([&kvCfg]{ return kvCfg.contains("sessions") && kvCfg.at("sessions").is_object(); }, "kv requires session settings") &&
                isValid([&kvCfg]{ return kvCfg.contains("save") && kvCfg.at("save").is_object(); }, "kv::save must be an object") &&
                validateSave(kvCfg.at("save"));
  
  if (valid)
  {
    const auto& seshCfg = kvCfg.at("sessions");

    valid = isValid([&seshCfg]{ return seshCfg.contains("enabled") && seshCfg.at("enabled").is_bool(); }, "sessions::enabled must be bool") &&
            isValid([&seshCfg]{ return seshCfg.contains("save") && seshCfg.at("save").is_object(); }, "sessions::save must be an object") &&
            validateSave(seshCfg.at("save"));
  }
  
  return valid;
}


bool parseTs (njson& cfg)
{
  const auto& tsCfg = cfg.at("ts");

  return isValid([&tsCfg]{ return tsCfg.is_object() && tsCfg.empty(); }, "ts must be an empty object");
}


NemesisConfig parse(std::filesystem::path path)
{
  NemesisConfig config; // default is an invalid state

  try
  {
    std::ifstream configStream{path};
    njson cfg = njson::parse(configStream);

    bool valid =  isValid([&cfg]{ return cfg.contains("version") && cfg.at("version").is_uint64(); },     "Require version as an unsigned int") &&
                  isValid([&cfg]{ return cfg.contains("mode") && cfg.at("mode").is_string(); },           "Mode must be \"kv\" or \"ts\"") &&
                  isValid([&cfg]{ return cfg.at("mode") == "kv" || cfg.at("mode") == "ts"; },             "Mode must be \"kv\" or \"ts\"") &&
                  isValid([&cfg]{ return cfg.at("version") == nemesis::core::NEMESIS_CONFIG_VERSION; },   "Config version not compatible") &&
                  isValid([&cfg]{ return !cfg.contains("core") || (cfg.contains("core") && cfg.at("core").is<std::size_t>()); },  "'core' must be an unsigned integer") &&
                  isValid([&cfg]{ return cfg.contains("ip") && cfg.contains("port") && cfg.contains("maxPayload"); },             "Require ip, port, maxPayload and save") &&
                  isValid([&cfg]{ return cfg.at("ip").is_string() && cfg.at("port").is_uint64() && cfg.at("maxPayload").is_uint64(); }, "ip must string, port and maxPayload must be unsigned integer") &&
                  isValid([&cfg]{ return cfg.at("maxPayload") >= nemesis::core::NEMESIS_KV_MINPAYLOAD; }, "maxPayload below minimum") &&
                  isValid([&cfg]{ return cfg.at("maxPayload") <= nemesis::core::NEMESIS_KV_MAXPAYLOAD; }, "maxPayload exceeds maximum") ;

    if (valid)
    {
      if (cfg.at("mode") == "kv")
        valid = parseKv(cfg);
      else if (cfg.at("mode") == "ts")
        valid = parseTs(cfg);
      
      if (valid)
        config = NemesisConfig{std::move(cfg)};
    }
  }
  catch(const std::exception& e)
  {
    PLOGF << "Config file not valid JSON";
  }

  return config;
}


NemesisConfig readConfig (const int argc, char ** argv)
{
  namespace po = boost::program_options;

  std::filesystem::path cfgPath;
  std::string loadName, loadPath;

  po::options_description common("");
  common.add_options()("help, h",    "Show help");

  po::options_description configFile("Config file");
  configFile.add_options()("config",  po::value<std::filesystem::path>(&cfgPath), "Path to json config file");

  po::options_description load("Load data");
  load.add_options()("loadPath",  po::value<std::string>(&loadPath), "Path to where to find loadName. If not set, will use kv::save::path from config");
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
  catch(po::error_with_option_name pex)
  {
    PLOGF << pex.what();
  }
  catch(...)
  {
    PLOGF << "Unknown error reading program options";
  }
  

  NemesisConfig config;

  if (parsedArgs)
  {
    if (vm.contains("help"))
      std::cout << all << '\n'; // intentional cout
    else
    { 
      PLOGI << "Reading config";

      if (vm.count("config") != 1U)
        std::cout << "Must set one --config option, with path to the JSON config file\n";
      else
      {
        if (!std::filesystem::exists(cfgPath))
          PLOGF << "Config file path not found";
        else if (config = parse(cfgPath); config.valid)
        {
          if (vm.count("loadPath") || vm.count("loadName"))
          {            
            config.loadName = loadName;
            config.loadPath = vm.count("loadPath") ? loadPath : NemesisConfig::kvSavePath(config.cfg);

            if (!std::filesystem::exists(config.loadPath))
            {
              PLOGF << "Load path does not exist: " << config.loadPath ;
              config = NemesisConfig{};
            }
          }
        } 
      }
    }
  }

  return config;
}



} // namespace core
} // namespace nemesis

#endif

