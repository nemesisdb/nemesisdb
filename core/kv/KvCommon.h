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
    KvArrayAppend,
    MAX,
    InternalSessionMonitor,
    InternalLoad,
    Unknown,
  };

  enum class ShQueryType : std::uint8_t
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
    MAX,
    InternalSessionMonitor,
    InternalLoad,
    Unknown,
  };


  enum class SaveDataType
  {
    RawKv,
    SessionKv
  };


  struct DataLoadPaths
  {
    fs::path root;
    fs::path md;
    fs::path data;
    bool valid{false};
  };


  // inline void setToken (njson& rsp, const SessionToken& token)
  // {
  //   rsp["tkn"] = token;
  // }


  struct LoadResult
  {  
    RequestStatus status;
    std::size_t nSessions{0};
    std::size_t nKeys{0};
    NemesisClock::duration duration{0};
  };




  ndb_always_inline SessionToken createSessionToken(const SessionName& name)
  {
    static const std::size_t seed = 99194853094755497U;
    const auto hash = std::hash<SessionName>{}(name);
    return (hash | seed);
  }


  ndb_always_inline SessionToken createSessionToken()
  {
    static UUIDv4::UUIDGenerator<std::mt19937_64> uuidGenerator; 
    const auto uuid = uuidGenerator.getUUID();
    return uuid.hash();
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
      // they are named with the timestamp, so find the highest (most recent)
      for (const auto& dir : fs::directory_iterator(loadRoot))
      {
        if (dir.is_directory())
          max = std::max<std::size_t>(std::stoul(dir.path().filename()), max);
      }

      const fs::path root {loadRoot / std::to_string(max)};
      return DataLoadPaths {.root = root, .md = root / "md" / "md.json", .data = root / "data", .valid = true};
    }
  }


  std::tuple<bool, const std::string_view> validatePreLoad (const std::string& loadName, const fs::path& persistPath, const bool sessionsEnabled)
  {
    if (!fs::exists(persistPath / loadName))
      return {false, "Load name does not exist"};
    else if (const auto [root, mdFile, data, valid] = getLoadPaths(persistPath / loadName); !valid)
      return {false, "Failed to get load paths"};
    else if (!fs::exists(root))
      return {false, "Load root does not exist"};
    else if (!fs::exists(mdFile))
        return {false, "Metadata does not exist"};
    else if (!(fs::exists(data) && fs::is_directory(data)))
      return {false, "Data directory does not exist or is not a directory"}; 
    else
    {
      PLOGI << "Reading metadata in " << mdFile;

      std::ifstream mdStream {mdFile};
      const auto mdJson = njson::parse(mdStream);
      const auto saveType = static_cast<SaveDataType>(mdJson.at("saveDataType").as<unsigned int>()) ;

      if (!mdJson.contains("status") || !mdJson.at("status").is_uint64() || !mdJson.contains("saveDataType"))
        return {false, "Metadata file invalid"};
      else if (mdJson["status"] != toUnderlying(KvSaveStatus::Complete))
        return {false, "Cannot load: save was incomplete"};
      else if (sessionsEnabled && saveType != SaveDataType::SessionKv)
        return {false, "Cannot load: server has sessions enabled but data being loaded is not session data"};
      else if (!sessionsEnabled && saveType != SaveDataType::RawKv)
        return {false, "Cannot load: server has sessions disabled but data being loaded is session data"};
      else
        return {true, ""};
    }
  }


  // MOVED

  template<typename T, std::size_t Size>
  struct PmrResource
  {
    PmrResource() : mbr(std::data(buffer), std::size(buffer)), alloc(&mbr)
    {

    }

    std::pmr::polymorphic_allocator<T>& getAlloc ()
    {
      return alloc;
    }

    private:
      std::array<std::byte, Size> buffer;
      std::pmr::monotonic_buffer_resource mbr;
      std::pmr::polymorphic_allocator<T> alloc;
  };


  ndb_always_inline void send (KvWebSocket * ws, const njson& msg)
  {
    ws->send(msg.to_string(), WsSendOpCode);
  }


  template<typename Json>
  bool doIsValid (const std::string_view queryRspName, KvWebSocket * ws, 
                  const Json& cmd, const std::map<const std::string_view, const Param>& params,
                  std::function<std::tuple<RequestStatus, const std::string_view>(const Json&)> onPostValidate = nullptr)
  {
    const auto [stat, msg] = isCmdValid<Json, RequestStatus,
                                              RequestStatus::Ok,
                                              RequestStatus::ParamMissing,
                                              RequestStatus::ValueTypeInvalid>(cmd, params, onPostValidate);
    
    if (stat != RequestStatus::Ok)
    {
      PLOGD << msg;
      send(ws, createErrorResponse(queryRspName, stat, msg));
    }
      
    return stat == RequestStatus::Ok;
  }


  bool isValid (const std::string_view queryRspName, KvWebSocket * ws, 
                const njson& cmd, const std::map<const std::string_view, const Param>& params,
                typename std::function<std::tuple<RequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
  {
    const auto& cmdRoot = cmd.object_range().cbegin()->value();
    return doIsValid(queryRspName, ws, cmdRoot, params, onPostValidate);
  }


  bool isValid (const std::string_view queryRspName, const std::string_view child, KvWebSocket * ws, 
                const njson& cmd, const std::map<const std::string_view, const Param>& params,
                std::function<std::tuple<RequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
  {
    const auto& childObject = cmd.object_range().cbegin()->value()[child];
    return doIsValid(queryRspName, ws, childObject, params, onPostValidate);
  }


  njson createInitialSaveMetaData(const std::string_view name, const bool sessionsEnabled)
  {
    njson metadata;
    metadata["name"] = name;
    metadata["version"] = METADATA_VERSION;
    metadata["status"] = toUnderlying(KvSaveStatus::Pending);        
    metadata["start"] = std::chrono::time_point_cast<KvSaveMetaDataUnit>(KvSaveClock::now()).time_since_epoch().count();
    metadata["complete"] = 0; // time completed
    metadata["saveDataType"] = sessionsEnabled ? toUnderlying(SaveDataType::SessionKv) : toUnderlying(SaveDataType::RawKv);
      
    return metadata;
  }


  void completeSaveMetaData(njson& metadata, const KvSaveStatus status)
  {
    metadata["status"] = toUnderlying(status);
    metadata["complete"] = chrono::time_point_cast<KvSaveMetaDataUnit>(KvSaveClock::now()).time_since_epoch().count();
  }
}
}
}

#endif
