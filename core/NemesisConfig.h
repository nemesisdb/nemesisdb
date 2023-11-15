#ifndef NDB_CORE_FUSIONCONFIG_H
#define NDB_CORE_FUSIONCONFIG_H

#include <string_view>
#include <mutex>
#include <fstream>
#include <filesystem>
#include <boost/program_options.hpp>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core {


struct NemesisConfig
{
  NemesisConfig() : valid(false)
  {

  }

  NemesisConfig(njson&& config) : cfg(std::move(config)), valid(true)
  {

  }

  NemesisConfig(const std::string_view config) : valid(true)
  {
    cfg = std::move(njson::parse(config));
  }

  
  static std::string kvSavePath (const njson& cfg)
  {
    return cfg.at("kv").at("save").at("path").as_string();
  }

  static bool kvSaveEnabled (const njson& cfg)
  {
    return cfg.at("kv").at("save").at("enabled").as_bool();
  }

  njson cfg;
  bool valid;
} ;


inline bool isValid (std::function<bool()> isValidCheck, const std::string_view msg)
{
  const auto valid = isValidCheck();
  if (!valid)
    std::cout << "\n** Config Error **\n" << msg << "\n****\n";
  return valid;
};


NemesisConfig readConfig(std::filesystem::path path)
{
  std::ifstream configStream{path};

  try
  {
    njson cfg = njson::parse(configStream);

    bool valid = false;
    valid = isValid([&cfg](){ return cfg.contains("version") && cfg.at("version").is_uint64(); }, "Require version as an unsigned int") &&
            isValid([&cfg](){ return cfg.at("version") == nemesis::core::NEMESIS_CONFIG_VERSION; }, "Config version not compatible") &&
            isValid([&cfg](){ return cfg.contains("kv") && cfg.at("kv").is_object(); }, "Require kv section as an object");
    
    if (valid)
    {
      const auto& kvCfg = cfg.at("kv");
      valid = isValid([&kvCfg](){ return kvCfg.contains("ip") && kvCfg.contains("port") && kvCfg.contains("maxPayload") && kvCfg.contains("save"); }, "kv section requires ip, port, maxPayload and save") &&
              isValid([&kvCfg](){ return kvCfg.at("ip").is_string() && kvCfg.at("port").is_uint64() && kvCfg.at("maxPayload").is_uint64() && kvCfg.at("save").is_object(); }, "kv::ip must string, kv::port and kv::maxPayload must be unsigned integer, kv::save must be an object") &&
              isValid([&kvCfg](){ return kvCfg.at("maxPayload") >= nemesis::core::NEMESIS_KV_MINPAYLOAD; }, "kv::maxPayload below minimum") &&
              isValid([&kvCfg](){ return kvCfg.at("maxPayload") <= nemesis::core::NEMESIS_KV_MAXPAYLOAD; }, "kv::maxPayload exceeds maximum");

      if (valid)
      {
        auto& saveCfg = kvCfg.at("save");
        valid = isValid([&saveCfg](){ return saveCfg.contains("path") && saveCfg.at("path").is_string(); }, "kv::save::path must be a string") &&
                isValid([&saveCfg](){ return saveCfg.contains("enabled") && saveCfg.at("enabled").is_bool(); }, "kv::save::enabled must be a bool");
      }

      if (valid)
        return NemesisConfig{std::move(cfg)};
    }
  }
  catch(const std::exception& e)
  {
    std::cout << "Config file not valid JSON\n";
  }  

  return NemesisConfig{};
}


NemesisConfig readConfig (const int argc, char ** argv)
{
  namespace po = boost::program_options;

  std::cout << "Reading config\n";

  std::filesystem::path cfgPath;

  po::options_description common("");
  common.add_options()("help, h",    "Show help");

  po::options_description configFile("Config file");
  configFile.add_options()("config",  po::value<std::filesystem::path>(&cfgPath),     "Path to json config file");
  
  po::options_description all;
  all.add(common);
  all.add(configFile);
  
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, all), vm);
  po::notify(vm);

  if (vm.contains("help"))
    std::cout << all << '\n';
  else
  { 
    if (vm.count("config") != 1U)
      std::cout << "Must set one --config option, with path to the JSON config file\n";
    else
    {
      if (!std::filesystem::exists(cfgPath))
        std::cout << "Config file path not found\n";
      else
        return readConfig(cfgPath);
    }
  }

  return {};
}



} // namespace core
} // namespace fusion

#endif
