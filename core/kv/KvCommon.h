#ifndef NDB_CORE_KVCOMMON_H
#define NDB_CORE_KVCOMMON_H

#include <map>
#include <array>
#include <any>
#include <ankerl/unordered_dense.h>
#include <uuid_v4/uuid_v4.h>
#include <uwebsockets/App.h>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core { namespace kv {


//inline ServerStats * serverStats;
inline const std::int16_t METADATA_VERSION = 2;


enum class KvQueryType : std::uint8_t
{ 
  ShNew,
  ShEnd,
  ShOpen,
  ShInfo,
  ShInfoAll,
  ShSave,
  ShLoad,
  ShEndAll,
  ShExists,
  KvSet,
  KvSetQ,
  KvGet,
  KvAdd,
  KvAddQ,
  KvRemove,
  KvClear,
  KvCount,
  KvContains,
  KvFind,
  KvUpdate,
  KvKeys,
  KvClearSet,
  KvSave,
  KvLoad,
  MAX,
  InternalSessionMonitor,
  InternalLoad,
  Unknown,
};

// This should be SaveType but that used originally for AllSessions or SelectSessions 
// which isn't necessary
enum class SaveDataType
{
  RawKv,
  SessionKv
};


// TODO this can be moved to the KvHandler with pmr, unless it's used elsewhere
const ankerl::unordered_dense::map<std::string_view, KvQueryType> QueryNameToType = 
{  
  // session
  {"SH_NEW",          KvQueryType::ShNew},
  {"SH_END",          KvQueryType::ShEnd},
  {"SH_OPEN",         KvQueryType::ShOpen},
  {"SH_INFO",         KvQueryType::ShInfo},
  {"SH_INFO_ALL",     KvQueryType::ShInfoAll},
  {"SH_SAVE",         KvQueryType::ShSave},
  {"SH_LOAD",         KvQueryType::ShLoad},
  {"SH_END_ALL",      KvQueryType::ShEndAll},
  {"SH_EXISTS",       KvQueryType::ShExists},
  // kv
  {"KV_SET",          KvQueryType::KvSet},
  {"KV_SETQ",         KvQueryType::KvSetQ},
  {"KV_GET",          KvQueryType::KvGet},
  {"KV_ADD",          KvQueryType::KvAdd},
  {"KV_ADDQ",         KvQueryType::KvAddQ},
  {"KV_RMV",          KvQueryType::KvRemove},
  {"KV_CLEAR",        KvQueryType::KvClear},
  {"KV_COUNT",        KvQueryType::KvCount},
  {"KV_CONTAINS",     KvQueryType::KvContains},
  {"KV_FIND",         KvQueryType::KvFind},
  {"KV_UPDATE",       KvQueryType::KvUpdate},
  {"KV_KEYS",         KvQueryType::KvKeys},
  {"KV_CLEAR_SET",    KvQueryType::KvClearSet},
  {"KV_SAVE",         KvQueryType::KvSave},
  {"KV_LOAD",         KvQueryType::KvLoad}
};


struct DataLoadPaths
{
  fs::path root;
  fs::path md;
  fs::path data;
  bool valid{false};
};


inline void setToken (njson& rsp, const SessionToken& token)
{
  rsp["tkn"] = token;
}


struct LoadResult
{  
  RequestStatus status;
  std::size_t nSessions{0};
  std::size_t nKeys{0};
  NemesisClock::duration duration{0};
};




ndb_always_inline SessionToken createSessionToken(const SessionName& name, const bool shared)
{
  if (shared)
  {
    static const std::size_t seed = 99194853094755497U;
    const auto hash = std::hash<SessionName>{}(name);
    return (hash | seed);
  }
  else
  {
    static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator; 
    const auto uuid = uuidGenerator.getUUID();
    return uuid.hash();
  }
}


uuid createUuid ()
{
  static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator; 
  
  uuid s;
  uuidGenerator.getUUID().str(s);
  return s;
}


DataLoadPaths getLoadPaths(const std::filesystem::path& loadRoot)
{
  std::size_t max = 0;

  if (countFiles(loadRoot) == 0)
    return {.valid = false};
  else
  {
    // loadRoot may contain several saves (i.e. SH_SAVE/KV_SAVE used multiple times with the same 'name'),
    // so use getDefaultDataSetPath() to get the most recent (until letting user select)
    for (const auto& dir : fs::directory_iterator(loadRoot))
    {
      if (dir.is_directory())
        max = std::max<std::size_t>(std::stoul(dir.path().filename()), max);
    }

    fs::path root {loadRoot / std::to_string(max)};
    return DataLoadPaths {.root = root, .md = root / "md" / "md.json", .data = root / "data", .valid = true};
  }
}


std::tuple<bool, const std::string_view> validatePreLoad (const std::string& loadName, const fs::path& loadPath, const bool sessionsEnabled)
{
  if (!fs::exists(loadPath / loadName))
    return {false, "Load name does not exist"};
  else if (const auto [root, md, data, valid] = getLoadPaths(loadPath / loadName); !valid)
    return {false, "Failed to get load paths"};
  else
  {
    if (!fs::exists(root))
      return {false, "Load root does not exist"};
    else if (!fs::exists(md))
      return {false, "Metadata does not exist"};
    else
    {
      PLOGI << "Reading metadata in " << md;

      std::ifstream mdStream {md};
      const auto mdJson = njson::parse(mdStream);

      if (!mdJson.contains("status") || !mdJson.at("status").is_uint64() || !mdJson.contains("saveDataType"))
        return {false, "Metadata file invalid"};
      else if (mdJson["status"] != toUnderlying(KvSaveStatus::Complete))
        return {false, "Cannot load: save was incomplete"};
      else if ((sessionsEnabled && static_cast<SaveDataType>(mdJson.at("saveDataType").as<unsigned int>()) != SaveDataType::SessionKv) || 
              (!sessionsEnabled && static_cast<SaveDataType>(mdJson.at("saveDataType").as<unsigned int>()) != SaveDataType::RawKv))
        return {false, "Cannot load: KV data not compatible with server sessions setting"};
      else if (!(fs::exists(data) && fs::is_directory(data)))
        return {false, "Data directory does not exist or is not a directory"}; 
      else
        return {true, ""};
    }
  }
}


// TODO this isn't used, but really should be
/*
ndb_always_inline bool valueTypeValid (const njson& value)
{
  static const std::set<jsoncons::json_type> DisallowedTypes = 
  {
    jsoncons::json_type::byte_string_value
  };

  if (value.is_array())
  {
    static const std::set<jsoncons::json_type> DisallowedTypes = 
    {
      jsoncons::json_type::byte_string_value
    };

    for(const auto& item : value.array_range())
    {
      if (item.type() == jsoncons::json_type::byte_string_value)
        return false;
    }

    return true;
  }
  else
    return value.type() != jsoncons::json_type::byte_string_value;
}
*/


}
}
}

#endif
