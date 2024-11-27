#ifndef NDB_CORE_KVEXECUTOR_H
#define NDB_CORE_KVEXECUTOR_H

#include <functional>
#include <core/NemesisCommon.h>
#include <core/CacheMap.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvCommands.h>


namespace nemesis { namespace core { namespace kv {

using namespace nemesis::core;
using namespace nemesis::core::kv;



class KvExecutor
{
public:

  static Response set (CacheMap& map,  const njson& cmd)
  {
    Response response;
    response.rsp = njson {jsoncons::json_object_arg, {{SetRsp, jsoncons::json_object_arg_t{}}}};

    auto& body = response.rsp.at(SetRsp);

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

    return response;
  }


  static Response setQ (CacheMap& map,  const njson& cmd)
  {
    Response response;

    try
    {         
      for(const auto& kv : cmd["keys"].object_range())
        map.set(kv.key(), kv.value());     
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp = njson {jsoncons::json_object_arg, {{SetQRsp, jsoncons::json_object_arg_t{}}}};
      response.rsp[SetQRsp]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response get (CacheMap& map,  const njson& cmd)
  {
    static njson Prepared {jsoncons::json_object_arg, {{GetRsp, {jsoncons::json_object_arg,
                                                                  {
                                                                    {"st", toUnderlying(RequestStatus::Ok)},
                                                                    {"keys", njson{}} // initialise as an empty object
                                                                  }}}}};
    
    Response response;
    response.rsp = Prepared;

    auto& body = response.rsp.at(GetRsp);
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
    
    return response;
  }


  static Response add (CacheMap& map,  const njson& cmd)
  {
    Response response;
    response.rsp = njson {jsoncons::json_object_arg, {{AddRsp, jsoncons::json_object_arg_t{}}}};
    
    response.rsp[AddRsp]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      for(const auto& kv : cmd["keys"].object_range())
        map.add(kv.key(), kv.value());
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[AddRsp]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response addQ (CacheMap& map,  const njson& cmd)
  {
    Response response;

    try
    {
      for(auto& kv : cmd["keys"].object_range())
        map.add(kv.key(), kv.value());
    }
    catch (const std::exception& ex)
    {
      PLOGE << ex.what();
      response.rsp = njson {jsoncons::json_object_arg, {{AddQRsp, jsoncons::json_object_arg_t{}}}};
      response.rsp[AddQRsp]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response remove (CacheMap& map,  const njson& cmd)
  {
    Response response;
    response.rsp[RmvRsp]["st"] = toUnderlying(RequestStatus::Ok);

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
      PLOGE << e.what();
      response.rsp[RmvRsp]["st"] = toUnderlying(RequestStatus::Unknown);
    }
    
    return response;
  }


  static Response clear (CacheMap& map,  const njson& cmd)
  {
    const auto[ok, count] = map.clear();
    
    Response response;
    
    response.rsp[ClearRsp]["st"] = ok ? toUnderlying(RequestStatus::Ok) : toUnderlying(RequestStatus::Unknown);
    response.rsp[ClearRsp]["cnt"] = count;
    
    return response;
  }


  static Response contains (CacheMap& map,  const njson& cmd)
  {
    Response response;
    response.rsp[ContainsRsp]["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp[ContainsRsp]["contains"] = njson::array{};

    auto& contains = response.rsp.at(ContainsRsp).at("contains");

    for (auto&& item : cmd["keys"].array_range())
    {
      if (item.is_string())
      {
        if (cachedkey key = item.as_string(); map.contains(key))
          contains.emplace_back(std::move(key));
      }
    }

    return response;
  }

  
  static Response find (CacheMap& map,  const njson& cmd)
  {
    const bool paths = cmd.at("rsp") == "paths";

    Response response;

    njson result = njson::array();

    if (!map.find(cmd, paths, result))
      response.rsp[FindRsp]["st"] = toUnderlying(RequestStatus::PathInvalid);
    else
    {       
      response.rsp[FindRsp]["st"] = toUnderlying(RequestStatus::Ok);

      if (cmd.at("rsp") != "kv")
        response.rsp[FindRsp][paths ? "paths" : "keys"] = std::move(result);
      else
      {
        // return key-values the same as KV_GET
        response.rsp[FindRsp]["keys"] = njson::object();

        for(auto& item : result.array_range())
        {
          const auto& key = item.as_string();

          if (const auto value = map.get(key); value)
            response.rsp[FindRsp]["keys"][key] = (*value).get();
        } 
      }
    }

    return response;
  }


  static Response update (CacheMap& map,  const njson& cmd)
  {
    const auto& key = cmd.at("key").as_string();
    const auto& path = cmd.at("path").as_string();

    const auto [keyExists, count] = map.update(key, path, cachedvalue::parse(cmd.at("value").to_string()));
    
    Response response;
    response.rsp[UpdateRsp]["st"] = keyExists ? toUnderlying(RequestStatus::Ok) : toUnderlying(RequestStatus::KeyNotExist);
    response.rsp[UpdateRsp]["cnt"] = count;

    return response;
  }


  static Response keys (CacheMap& map,  const njson& cmd)
  {
    Response response;
    response.rsp[KeysRsp]["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp[KeysRsp]["keys"] = std::move(map.keys());
    return response;
  }


  static Response clearSet (CacheMap& map,  const njson& cmd)
  {
    Response response;
    response.rsp[ClearSetRsp]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      if (const auto[valid, size] = map.clear(); valid)
      {
        response.rsp[ClearSetRsp]["cnt"] = size;

        for(const auto& kv : cmd["keys"].object_range())
          map.set(kv.key(), kv.value());
      }
      else
      {
        PLOGE << "KV_CLEAR_SET failed to clear";
        response.rsp[ClearSetRsp]["st"] = toUnderlying(RequestStatus::Unknown);
        response.rsp[ClearSetRsp]["cnt"] = 0U;
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[ClearSetRsp]["st"] = toUnderlying(RequestStatus::Unknown);
      response.rsp[ClearSetRsp]["cnt"] = 0U;
    }

    return response;
  }


  static Response count (CacheMap& map,  const njson& cmd)
  {
    Response response;
    response.rsp[CountRsp]["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp[CountRsp]["cnt"] = map.count();
    return response;
  }


  static Response arrayAppend (CacheMap& map,  const njson& cmd)
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

    Response response;
    response.rsp[ArrAppendRsp]["st"] = toUnderlying(st);
    return response;
  }


  static Response saveKv (const CacheMap& map, const fs::path& path, const std::string_view name)
  {
    static const std::streamoff MaxDataFileSize = 10U * 1024U * 1024U;

    auto start = NemesisClock::now();

    RequestStatus status = RequestStatus::SaveComplete;

    Response response;
    response.rsp[SaveRsp]["name"] = name;

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
    

    response.rsp[SaveRsp]["st"] = toUnderlying(status);
    response.rsp[SaveRsp]["duration"] = chrono::duration_cast<chrono::milliseconds>(NemesisClock::now() - start).count();

    return response;
  }


  static Response loadKv (const std::string& loadName, CacheMap& map, const fs::path& dataRoot)
  {
    RequestStatus status{RequestStatus::Loading};
    std::size_t nKeys{0};
    
    Response response;
    response.rsp[LoadRsp]["st"] = toUnderlying(status);
    response.rsp[LoadRsp]["name"] = loadName;
    response.rsp[LoadRsp]["duration"] = 0; // updated below
    response.rsp[LoadRsp]["sessions"] = 0; // always 0 when sessions disabled
    response.rsp[LoadRsp]["keys"] = 0; // updated below

    try
    {      
      const auto start = NemesisClock::now();

      for (const auto& kvFile : fs::directory_iterator{dataRoot})
      {
        status = readKvFile(map, kvFile.path(), nKeys);
        
        if (status == RequestStatus::LoadError)
          break;
      }

      response.rsp[LoadRsp]["duration"] = chrono::duration_cast<chrono::milliseconds>(NemesisClock::now() - start).count();
      response.rsp[LoadRsp]["keys"] = nKeys;      
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      status = RequestStatus::LoadError;
    }

    response.rsp[LoadRsp]["st"] = toUnderlying(status);
    
    return response;
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

