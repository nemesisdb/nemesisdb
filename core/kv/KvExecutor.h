

#ifndef NDB_CORE_KVEXECUTOR_H
#define NDB_CORE_KVEXECUTOR_H

#include <functional>
#include <string_view>
#include <core/NemesisCommon.h>
#include <core/CacheMap.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvCommands.h>
#include <core/sh/ShCommands.h>


namespace nemesis { namespace kv {

using namespace nemesis::core;

namespace shcmds = nemesis::sh::cmds;
namespace kvcmds = nemesis::kv::cmds;


// Manages store, retrieving, etc keys.
// The class is used for top level/independent keys which use the same map,
// but also sessions.
// Each function has a CacheMap param, which is either the top level map
// or the map from a session.
//
// RspMeta and KvOnlyMeta structs are used to build the response object,
// with the correct response name, either a KV or SH response.
template<bool WithSessions>
class KvExecutor
{
  template<bool Sessions, const char * kv, const char * sh>
  struct RspMeta
  {
    static constexpr auto name = sh;

    static Response make()
    { 
      static const Response r {.rsp = njson{jsoncons::json_object_arg, {{name, njson::object()}}}};
      return r;
    }
  };

  template<const char * kv, const char * sh>
  struct RspMeta<false, kv, sh>
  {
    static constexpr auto name = kv;

    static Response make()
    { 
      static const Response r {.rsp = njson{jsoncons::json_object_arg, {{name, njson::object()}}}};
      return r;
    }
  };

  template<const char * kv>
  struct KvOnlyMeta
  {
    static constexpr auto name = kv;

    static Response make()
    { 
      static const Response r {.rsp = njson{jsoncons::json_object_arg, {{name, njson::object()}}}};
      return r;
    }
  };


public:

  static Response set (CacheMap& map,  const njson& cmd)
  {
    using Rsp = RspMeta<WithSessions, kvcmds::SetRsp, shcmds::SetRsp>;

    Response response = Rsp::make();
    auto& body = response.rsp.at(Rsp::name);

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


  /*
  static Response setQ (CacheMap& map,  const njson& cmd)
  {
    using Rsp = KvOnlyMeta<kvcmds::GetRsp>;

    Response response = Rsp::make();

    try
    {         
      for(const auto& kv : cmd["keys"].object_range())
        map.set(kv.key(), kv.value());     
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp = njson {jsoncons::json_object_arg, {{Rsp::name, jsoncons::json_object_arg_t{}}}};
      response.rsp[Rsp::name]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }
  */


  static Response get (CacheMap& map,  const njson& cmd)
  {
    using Rsp = RspMeta<WithSessions, kvcmds::GetRsp, shcmds::GetRsp>;

    Response response = Rsp::make();

    auto& body = response.rsp.at(Rsp::name);
    body["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      body["keys"] = njson::object();
      auto& keys = body.at("keys");

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
    using Rsp = RspMeta<WithSessions, kvcmds::AddRsp, shcmds::AddRsp>;

    Response response = Rsp::make();

    auto& body = response.rsp.at(Rsp::name);   
    body["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      for(const auto& kv : cmd["keys"].object_range())
        map.add(kv.key(), kv.value());
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      body["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  /*
  static Response addQ (CacheMap& map,  const njson& cmd)
  {
    using Rsp = KvOnlyMeta<kvcmds::AddQRsp>;

    Response response;

    try
    {
      for(auto& kv : cmd["keys"].object_range())
        map.add(kv.key(), kv.value());
    }
    catch (const std::exception& ex)
    {
      PLOGE << ex.what();
      response = Rsp::make();
      response.rsp.at(Rsp::name)["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }
  */


  static Response remove (CacheMap& map,  const njson& cmd)
  {
    using Rsp = RspMeta<WithSessions, kvcmds::RmvRsp, shcmds::RmvRsp>;

    Response response = Rsp::make();

    auto& body = response.rsp.at(Rsp::name);   
    body["st"] = toUnderlying(RequestStatus::Ok);


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
      body["st"] = toUnderlying(RequestStatus::Unknown);
    }
    
    return response;
  }


  static Response clear (CacheMap& map,  const njson& cmd)
  {
    using Rsp = RspMeta<WithSessions, kvcmds::ClearRsp, shcmds::ClearRsp>;

    const auto[ok, count] = map.clear();

    Response response = Rsp::make();
    response.rsp.at(Rsp::name)["st"] = ok ? toUnderlying(RequestStatus::Ok) : toUnderlying(RequestStatus::Unknown);
    response.rsp.at(Rsp::name)["cnt"] = count;
    
    return response;
  }


  static Response contains (CacheMap& map,  const njson& cmd)
  {
    using Rsp = RspMeta<WithSessions, kvcmds::ContainsRsp, shcmds::ContainsRsp>;

    Response response = Rsp::make();

    response.rsp.at(Rsp::name)["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp.at(Rsp::name)["contains"] = njson::array{};

    auto& contains = response.rsp.at(Rsp::name).at("contains");

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
    using Rsp = KvOnlyMeta<kvcmds::FindRsp>;

    Response response = Rsp::make();

    const bool paths = cmd.at("rsp") == "paths";
    njson result = njson::array();

    auto& body = response.rsp.at(Rsp::name);


    if (!map.find(cmd, paths, result))
      body["st"] = toUnderlying(RequestStatus::PathInvalid);
    else
    {       
      body["st"] = toUnderlying(RequestStatus::Ok);

      if (cmd.at("rsp") != "kv")
        response.rsp.at(Rsp::name)[paths ? "paths" : "keys"] = std::move(result);
      else
      {
        // return key-values the same as KV_GET
        body["keys"] = njson::object();

        for(auto& item : result.array_range())
        {
          const auto& key = item.as_string();

          if (const auto value = map.get(key); value)
            body["keys"][key] = (*value).get();
        } 
      }
    }

    return response;
  }


  static Response update (CacheMap& map,  const njson& cmd)
  {
    using Rsp = KvOnlyMeta<kvcmds::UpdateRsp>;

    const auto& key = cmd.at("key").as_string();
    const auto& path = cmd.at("path").as_string();

    const auto [keyExists, count] = map.update(key, path, cachedvalue::parse(cmd.at("value").to_string()));
    
    Response response = Rsp::make();
    response.rsp.at(Rsp::name)["st"] = keyExists ? toUnderlying(RequestStatus::Ok) : toUnderlying(RequestStatus::KeyNotExist);
    response.rsp.at(Rsp::name)["cnt"] = count;

    return response;
  }


  static Response keys (CacheMap& map,  const njson& cmd)
  {
    using Rsp = RspMeta<WithSessions, kvcmds::KeysRsp, shcmds::KeysRsp>;

    Response response = Rsp::make();
    response.rsp.at(Rsp::name)["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp.at(Rsp::name)["keys"] = map.keys();
    return response;
  }


  static Response clearSet (CacheMap& map,  const njson& cmd)
  {
    using Rsp = RspMeta<WithSessions, kvcmds::ClearSetRsp, shcmds::ClearSetRsp>;

    Response response = Rsp::make();
    auto& body = response.rsp.at(Rsp::name);

    body["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      if (const auto[valid, size] = map.clear(); valid)
      {
        body["cnt"] = size;

        for(const auto& kv : cmd["keys"].object_range())
          map.set(kv.key(), kv.value());
      }
      else
      {
        PLOGE << Rsp::name << " failed to clear";
        body["st"] = toUnderlying(RequestStatus::Unknown);
        body["cnt"] = 0U;
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      body["st"] = toUnderlying(RequestStatus::Unknown);
      body["cnt"] = 0U;
    }

    return response;
  }


  static Response count (CacheMap& map,  const njson& cmd)
  {
    using Rsp = RspMeta<WithSessions, kvcmds::CountRsp, shcmds::CountRsp>;

    Response response = Rsp::make();
    response.rsp.at(Rsp::name)["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp.at(Rsp::name)["cnt"] = map.count();
    return response;
  }


  static Response arrayAppend (CacheMap& map,  const njson& cmd)
  {
    using Rsp = KvOnlyMeta<kvcmds::ArrAppendRsp>;


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

    Response response = Rsp::make();
    response.rsp[Rsp::name]["st"] = toUnderlying(st);
    return response;
  }


  static Response saveKv (const CacheMap& map, const fs::path& path, const std::string_view name)
  {
    static const std::streamoff MaxDataFileSize = 10U * 1024U * 1024U;

    auto start = NemesisClock::now();

    RequestStatus status = RequestStatus::SaveComplete;

    Response response;
    response.rsp[kvcmds::SaveRsp]["name"] = name;

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
    

    response.rsp[kvcmds::SaveRsp]["st"] = toUnderlying(status);
    response.rsp[kvcmds::SaveRsp]["duration"] = chrono::duration_cast<chrono::milliseconds>(NemesisClock::now() - start).count();

    return response;
  }


  static Response loadKv (const std::string& loadName, CacheMap& map, const fs::path& dataRoot)
  {
    RequestStatus status{RequestStatus::Loading};
    std::size_t nKeys{0};
    
    Response response;
    response.rsp[kvcmds::LoadRsp]["st"] = toUnderlying(status);
    response.rsp[kvcmds::LoadRsp]["name"] = loadName;
    response.rsp[kvcmds::LoadRsp]["duration"] = 0; // updated below
    response.rsp[kvcmds::LoadRsp]["sessions"] = 0; // always 0 when sessions disabled
    response.rsp[kvcmds::LoadRsp]["keys"] = 0; // updated below

    try
    {      
      const auto start = NemesisClock::now();

      for (const auto& kvFile : fs::directory_iterator{dataRoot})
      {
        status = readKvFile(map, kvFile.path(), nKeys);
        
        if (status == RequestStatus::LoadError)
          break;
      }

      response.rsp[kvcmds::LoadRsp]["duration"] = chrono::duration_cast<chrono::milliseconds>(NemesisClock::now() - start).count();
      response.rsp[kvcmds::LoadRsp]["keys"] = nKeys;      
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      status = RequestStatus::LoadError;
    }

    response.rsp[kvcmds::LoadRsp]["st"] = toUnderlying(status);
    
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

#endif

