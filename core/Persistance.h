#ifndef NDB_CORE_PERSISTANCE_H
#define NDB_CORE_PERSISTANCE_H

#include <fstream>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core { 


inline const std::int16_t METADATA_VERSION = 2;


enum class SaveDataType
{
  RawKv,
  SessionKv
};


enum class KvSaveStatus
{
  Pending = 0,
  Complete,
  Error
};


struct DataLoadPaths
{
  fs::path root;
  fs::path md;
  fs::path data;
  bool valid{false};
};


struct LoadResult
{  
  RequestStatus status;
  std::size_t nSessions{0};
  std::size_t nKeys{0};
  NemesisClock::duration duration{0};
};


DataLoadPaths getLoadPaths(const std::filesystem::path& loadRoot)
{
  auto countFiles = [](const fs::path& path) -> std::size_t
  {
    return (std::size_t)std::distance(fs::directory_iterator{path}, fs::directory_iterator{});
  };

  if (countFiles(loadRoot) == 0)
    return {.valid = false};
  else
  {
    std::size_t max = 0;

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


}
}

#endif
