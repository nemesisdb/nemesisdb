#ifndef NDB_CORE_KVHANDLER2_H
#define NDB_CORE_KVHANDLER2_H


#include <functional>
#include <tuple>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/Persistance.h>
#include <core/NemesisConfig.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvExecutor.h>
#include <core/kv/KvCommandValidate.h>
#include <core/kv/CacheMap.h>


namespace nemesis { namespace kv {


using namespace nemesis::kv::cmds;


/*
KvHandler receives a command:
  - checks if the command exists and is enabled
  - calls a handler function
*/
class KvHandler2
{
public:
  KvHandler2() : m_settings(Settings::get())
  {

  }


public:
   
  Response handle(const ndb::request::KVSet& set)
  { 
    const auto& vec = set.kv_flexbuffer_root().AsMap().Values();
    const auto& keys = set.kv_flexbuffer_root().AsMap().Keys();

    for (std::size_t i = 0 ; i < vec.size(); ++i)
    {
      const auto& key = keys[i].AsString().str();

      switch (vec[i].GetType())
      {
        using enum flexbuffers::Type;

        case FBT_INT:
          m_map.set<FBT_INT>(key, vec[i].AsInt64());
        break;
        
        case FBT_UINT:
          m_map.set<FBT_UINT>(key, vec[i].AsUInt64());
        break;

        case FBT_BOOL:
          m_map.set<FBT_BOOL>(key, vec[i].AsBool());
        break;

        case FBT_STRING:
          m_map.set<FBT_STRING>(key, vec[i].AsString().str());
        break;

        case FBT_FLOAT:
          m_map.set<FBT_FLOAT>(key, vec[i].AsFloat());
        break;

        default:
          PLOGE << __FUNCTION__ << " - unknown type";
        break;
      }
    }

    return Response{};
  }
  

  flatbuffers::FlatBufferBuilder handle(const ndb::request::KVGet& get)
  {
    flatbuffers::FlatBufferBuilder fbb;

    if (!get.keys())
    {
      auto rsp = ndb::response::CreateResponse( fbb,
                                                ndb::response::Status::Status_Fail,
                                                ndb::response::ResponseBody::ResponseBody_KVGet);

      fbb.Finish(rsp);
    }
    else
    {
      FlexBuilder flxb;
      m_map.get(*get.keys(), flxb);
      flxb.Finish();
      
      const auto buff = flxb.GetBuffer();

      const auto vec = fbb.CreateVector(buff);  // place the flex buffer vector in the flat buffer
      const auto body = ndb::response::CreateKVGet(fbb, vec);
      
      auto rsp = ndb::response::CreateResponse( fbb,
                                                ndb::response::Status::Status_Ok,
                                                ndb::response::ResponseBody::ResponseBody_KVGet,
                                                body.Union());

      fbb.Finish(rsp);
    }
   
    return fbb;
  }


  // Called when loading at startup
  LoadResult internalLoad(const std::string& loadName, const fs::path& dataSetsRoot)
  {
    const auto start = NemesisClock::now();
    
    // call doLoad(), which is used by KV_LOAD, grabbing from the rsp
    const njson rsp = doLoad(loadName, dataSetsRoot);

    LoadResult loadResult;
    loadResult.duration = NemesisClock::now() - start;
    loadResult.status = static_cast<RequestStatus>(rsp[kv::LoadRsp]["st"].as<std::uint64_t>());
    loadResult.nKeys = rsp[kv::LoadRsp]["keys"].as<std::size_t>();
    loadResult.nSessions = 0;
    
    return loadResult;
  }


private:
    
  /*
  Response validateAndExecute(const std::map<KvQueryType, ValidateExecute>::const_iterator it, const njson& request, const std::string_view reqName, const std::string_view rspName)
  {
    const auto& validateExecute = it->second;

    if (const auto status = validateExecute.validate(reqName, rspName, request); status != RequestStatus::Ok)
      return Response{.rsp = createErrorResponse(rspName, status)};
    else
    {
      const auto& body = request.at(reqName);
      return validateExecute.execute(m_map, body);
    }
  }
  */
  
  Response save(njson& request)
  {
    if (!m_settings.persistEnabled)
      return Response{.rsp = createErrorResponse(SaveRsp, RequestStatus::CommandDisabled)};
    else
    {
      if (const auto status = kv::validateSave(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(SaveRsp, status)};
      else
        return doSave(SaveRsp, request.at(SaveReq));
    }
  }


  Response load(njson& request)
  {
    if (const auto status = kv::validateLoad(request) ; status != RequestStatus::Ok)
      return Response{.rsp = createErrorResponse(LoadRsp, status)};
    else
    {
      const auto& loadName = request.at(LoadReq).at("name").as_string();

      if (const PreLoadInfo info = validatePreLoad(loadName, fs::path{m_settings.persistPath}); !info.valid)
      {
        PLOGE << info.err;

        Response response;
        response.rsp[LoadRsp]["st"] = toUnderlying(RequestStatus::LoadError);
        response.rsp[LoadRsp]["name"] = loadName;
        return response;
      }
      else
      {
        const auto [root, md, data, pathsValid] = getLoadPaths(fs::path{m_settings.persistPath} / loadName);        
        return Response {.rsp = doLoad(loadName, data)};
      }
    }
  }
  
  
  Response doSave (const std::string_view queryRspName, njson& cmd)
  {
    /*const auto& name = cmd.at("name").as_string();
    const auto dataSetDir = std::to_string(KvSaveClock::now().time_since_epoch().count());
    const auto root = fs::path {m_settings.persistPath} / name / dataSetDir;
    const auto metaPath = root / "md";
    const auto dataPath = root / "data";
    

    if (auto [preparedStatus, metaStream] = prepareSave(cmd, root); preparedStatus != RequestStatus::Ok)
    {
      return Response{.rsp = createErrorResponse(queryRspName, preparedStatus)};
    }
    else
    {
      auto metaData = createInitialSaveMetaData(metaStream, name, false);
      
      KvSaveStatus metaDataStatus = KvSaveStatus::Pending;
      Response response;

      try
      {
        response = KvExecutor::saveKv(m_map, dataPath, name);
        metaDataStatus = KvSaveStatus::Complete;
      }
      catch(const std::exception& e)
      {
        PLOGE << e.what();
        metaDataStatus = KvSaveStatus::Error;
        response.rsp[queryRspName]["st"] = toUnderlying(RequestStatus::SaveError);
      }
      
      // update metdata
      completeSaveMetaData(metaStream, metaData, metaDataStatus);
      
      return response;
    }
    */
   return Response{};
  }
  

  njson doLoad (const std::string& loadName, const fs::path& dataSetsRoot)
  {
    /*PLOGI << "Loading from " << dataSetsRoot;
    
    Response response = KvExecutor::loadKv (loadName, m_map, dataSetsRoot);

    PLOGI << "Loading complete";

    return response.rsp;*/
    return njson{};
  }


private:
  const Settings& m_settings;
  kv::CacheMap m_map;
};

}
}

#endif
