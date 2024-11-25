#ifndef NDB_CORE_KVEXECUTOR_H
#define NDB_CORE_KVEXECUTOR_H

#include <functional>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvSessions.h>


namespace nemesis { namespace core { namespace kv {

using namespace nemesis::core;
using namespace nemesis::core::kv;



class KvExecutor
{
public:

  static njson set (CacheMap& map,  const njson& cmd)
  {
    njson rsp {jsoncons::json_object_arg, {{"KV_SET_RSP", jsoncons::json_object_arg_t{}}}};
    auto& body = rsp.at("KV_SET_RSP");

    body["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      for(const auto& kv : cmd["keys"].object_range())
        map.set(kv.key(), kv.value());
    }
    catch(const std::exception& ex)
    {
      PLOGE << ex.what();
      body["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return rsp;
  }


  static njson setQ (CacheMap& map,  const njson& cmd)
  {
    njson rsp;

    try
    {         
      for(const auto& kv : cmd["keys"].object_range())
        map.set(kv.key(), kv.value());     
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      rsp = njson {jsoncons::json_object_arg, {{"KV_SETQ_RSP", jsoncons::json_object_arg_t{}}}};
      rsp["KV_SETQ_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return rsp;
  }


  static njson get (CacheMap& map,  const njson& cmd)
  {
    static njson Prepared {jsoncons::json_object_arg, {{"KV_GET_RSP", {jsoncons::json_object_arg,
                                                                      {
                                                                        {"st", toUnderlying(RequestStatus::Ok)},
                                                                        {"keys", njson{}} // initialise as an empty object
                                                                      }}}}};
    
    njson rsp = Prepared;
    auto& body = rsp.at("KV_GET_RSP");
    auto& keys = body.at("keys");

    try
    {
      for(const auto& item : cmd["keys"].array_range())
      {
        if (item.is_string()) [[likely]]
        {
          const auto& key = item.as_string();

          if (const auto value = map.get(key) ; value)
            keys.try_emplace(key, (*value).get());
        }
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      body["st"] = toUnderlying(RequestStatus::Unknown);
    }
    
    return rsp;
  }


  static njson add (CacheMap& map,  const njson& cmd)
  {
    njson rsp {jsoncons::json_object_arg, {{"KV_ADD_RSP", jsoncons::json_object_arg_t{}}}};
    
    rsp["KV_ADD_RSP"]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      for(const auto& kv : cmd["keys"].object_range())
        map.add(kv.key(), kv.value());
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      rsp["KV_ADD_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return rsp;
  }


  static njson addQ (CacheMap& map,  const njson& cmd)
  {
    njson rsp;

    try
    {
      for(auto& kv : cmd["keys"].object_range())
        map.add(kv.key(), kv.value());
    }
    catch (const std::exception& ex)
    {
      PLOGE << ex.what();
      rsp = njson {jsoncons::json_object_arg, {{"KV_ADDQ_RSP", jsoncons::json_object_arg_t{}}}};
      rsp["KV_ADDQ_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return rsp;
  }


  static njson remove (CacheMap& map,  const njson& cmd)
  {
    njson rsp;
    rsp["KV_RMV_RSP"]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      for(const auto& value : cmd["keys"].array_range())
      {
        if (value.is_string())
        {
          const auto& key = value.as_string();
          map.remove(key);
        }
      }  
    }
    catch(const std::exception& e)
    {
      rsp["KV_RMV_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
      PLOGE << e.what();
    }
    
    return rsp;
  }


  static njson clear (CacheMap& map,  const njson& cmd)
  {
    const auto[ok, count] = map.clear();

    njson rsp;
    rsp["KV_CLEAR_RSP"]["st"] = ok ? toUnderlying(RequestStatus::Ok) : toUnderlying(RequestStatus::Unknown);
    rsp["KV_CLEAR_RSP"]["cnt"] = count;
    
    return rsp;
  }


  static njson contains (CacheMap& map,  const njson& cmd)
  {
    njson rsp;
    rsp["KV_CONTAINS_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["KV_CONTAINS_RSP"]["contains"] = njson::array{};

    auto& contains = rsp.at("KV_CONTAINS_RSP").at("contains");

    for (auto&& item : cmd["keys"].array_range())
    {
      if (item.is_string())
      {
        if (cachedkey key = item.as_string(); map.contains(key))
          contains.emplace_back(std::move(key));
      }
    }

    return rsp;
  }

  
  static njson find (CacheMap& map,  const njson& cmd)
  {
    const bool paths = cmd.at("rsp") == "paths";

    njson rsp;

    njson result = njson::array();

    if (!map.find(cmd, paths, result))
      rsp["KV_FIND_RSP"]["st"] = toUnderlying(RequestStatus::PathInvalid);
    else
    {       
      rsp["KV_FIND_RSP"]["st"] = toUnderlying(RequestStatus::Ok);

      if (cmd.at("rsp") != "kv")
        rsp["KV_FIND_RSP"][paths ? "paths" : "keys"] = std::move(result);
      else
      {
        // return key-values the same as KV_GET
        rsp["KV_FIND_RSP"]["keys"] = njson::object();

        for(auto& item : result.array_range())
        {
          const auto& key = item.as_string();

          if (const auto value = map.get(key); value)
            rsp["KV_FIND_RSP"]["keys"][key] = (*value).get();
        } 
      }
    }

    return rsp;
  }


  static njson update (CacheMap& map,  const njson& cmd)
  {
    const auto& key = cmd.at("key").as_string();
    const auto& path = cmd.at("path").as_string();

    const auto [keyExists, count] = map.update(key, path, cachedvalue::parse(cmd.at("value").to_string()));
    
    njson rsp;
    rsp["KV_UPDATE_RSP"]["st"] = keyExists ? toUnderlying(RequestStatus::Ok) : toUnderlying(RequestStatus::KeyNotExist);
    rsp["KV_UPDATE_RSP"]["cnt"] = count;

    return rsp;
  }


  static njson keys (CacheMap& map,  const njson& cmd)
  {
    njson rsp;
    rsp["KV_KEYS_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["KV_KEYS_RSP"]["keys"] = std::move(map.keys());
    return rsp;
  }


  static njson clearSet (CacheMap& map,  const njson& cmd)
  {
    njson rsp;
    rsp["KV_CLEAR_SET_RSP"]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      if (const auto[valid, size] = map.clear(); valid)
      {
        rsp["KV_CLEAR_SET_RSP"]["cnt"] = size;

        for(const auto& kv : cmd["keys"].object_range())
          map.set(kv.key(), kv.value());
      }
      else
      {
        PLOGE << "KV_CLEAR_SET failed to clear";
        rsp["KV_CLEAR_SET_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
        rsp["KV_CLEAR_SET_RSP"]["cnt"] = 0U;
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      rsp["KV_CLEAR_SET_RSP"]["st"] = toUnderlying(RequestStatus::Unknown);
      rsp["KV_CLEAR_SET_RSP"]["cnt"] = 0U;
    }

    return rsp;
  }


  static njson count (CacheMap& map,  const njson& cmd)
  {
    njson rsp;
    rsp["KV_COUNT_RSP"]["st"] = toUnderlying(RequestStatus::Ok);
    rsp["KV_COUNT_RSP"]["cnt"] = map.count();
    return rsp;
  }


  static njson arrayAppend (CacheMap& map,  const njson& cmd)
  {
    const auto& key = cmd.at("key").as_string();
    const auto& items = cmd.at("items");
    
    RequestStatus st = RequestStatus::Ok;

    if (map.contains(key))
    {
      if (cmd.contains("name"))
        st = map.arrayAppend(key, cmd.at("name").as_string_view(), items) ? RequestStatus::Ok : RequestStatus::Unknown;
      else
        st = map.arrayAppend(key, items) ? RequestStatus::Ok : RequestStatus::Unknown;
    }
    else
      st = RequestStatus::KeyNotExist;

    njson rsp;
    rsp["KV_ARR_APPEND_RSP"]["st"] = toUnderlying(st);
    return rsp;
  }


  static njson saveKv (const CacheMap& map, const fs::path& path, const std::string_view name)
  {
    static const std::streamoff MaxDataFileSize = 10U * 1024U * 1024U;

    auto start = NemesisClock::now();

    RequestStatus status = RequestStatus::SaveComplete;

    njson rsp;
    rsp["KV_SAVE_RSP"]["name"] = name;

    try
    {
      if (!fs::create_directories(path))
        status = RequestStatus::SaveError;
      else
      {
        std::string buffer; // vector<char> ?
        buffer.reserve(MaxDataFileSize);

        std::stringstream sstream{buffer};
        std::size_t nFiles = 0;
        
        bool first = true;

        for(const auto& [k, v] : map.map())
        {   
          if (!first)
            sstream << ',';

          first = false;

          sstream << '"' << k << "\":" << v.to_string();
          
          if (sstream.tellp() >= MaxDataFileSize)
          {
            flushKvBuffer(path / std::to_string(nFiles), sstream);
            ++nFiles;
            first = true;
          }         
        }

        // leftovers that didn't reach the file size
        if (sstream.tellp() > 0)
          flushKvBuffer(path / std::to_string(nFiles), sstream);
      }
    }
    catch(const std::exception& e)
    {
      status = RequestStatus::SaveError;
    }
    

    rsp["KV_SAVE_RSP"]["st"] = toUnderlying(status);
    rsp["KV_SAVE_RSP"]["duration"] = chrono::duration_cast<chrono::milliseconds>(NemesisClock::now() - start).count();

    return rsp;
  }


  static njson loadKv (const std::string& loadName, CacheMap& map, const fs::path& dataRoot)
  {
    RequestStatus status{RequestStatus::Loading};
    std::size_t nKeys{0};
    
    njson rsp;
    rsp["KV_LOAD_RSP"]["st"] = toUnderlying(status);
    rsp["KV_LOAD_RSP"]["name"] = loadName;
    rsp["KV_LOAD_RSP"]["duration"] = 0; // updated below
    rsp["KV_LOAD_RSP"]["sessions"] = 0; // always 0 when sessions disabled
    rsp["KV_LOAD_RSP"]["keys"] = 0; // updated below

    try
    {      
      const auto start = NemesisClock::now();

      for (const auto& kvFile : fs::directory_iterator{dataRoot})
      {
        status = readKvFile(map, kvFile.path(), nKeys);
        
        if (status == RequestStatus::LoadError)
          break;
      }

      rsp["KV_LOAD_RSP"]["duration"] = chrono::duration_cast<chrono::milliseconds>(NemesisClock::now() - start).count();
      rsp["KV_LOAD_RSP"]["keys"] = nKeys;      
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      status = RequestStatus::LoadError;
    }

    rsp["KV_LOAD_RSP"]["st"] = toUnderlying(status);
    
    return rsp;
  }


private:

  static void flushKvBuffer (const fs::path& path, std::stringstream& sstream)
  {
    std::ofstream dataStream {path};
    dataStream << "{ \"keys\":{" << sstream.str() << "}}";

    sstream.clear();
    sstream.str(std::string{});
  }


  static RequestStatus readKvFile (CacheMap& map, const fs::path path, std::size_t& nKeys)
  {
    RequestStatus status = RequestStatus::LoadComplete;

    try
    {
      std::ifstream dataStream{path};
      
      if (const auto root = njson::parse(dataStream); root.is_object() && root.contains("keys") && root.at("keys").is_object())
      {
        for (const auto& item : root.at("keys").object_range())
        {
          map.set(item.key(), item.value());
          ++nKeys;
        }
      }      
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      status = RequestStatus::LoadError;
    }
    
    return status;
  }

};


}
}
}

#endif

