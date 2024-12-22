#ifndef NDB_CORE_KVHANDLERS_H
#define NDB_CORE_KVHANDLERS_H


#include <functional>
#include <tuple>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/Persistance.h>
#include <core/NemesisConfig.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvExecutor.h>
#include <core/kv/KvCommandValidate.h>



namespace nemesis { namespace kv {


using namespace nemesis::kv::cmds;


/*
KvHandler receives a command:
  - checks if the command exists and is enabled
  - calls a handler function
*/
class KvHandler
{
public:
  KvHandler() : m_settings(Settings::get())
  {

  }

private:

  using HandlerPmrMap = ankerl::unordered_dense::pmr::map<KvQueryType, Handler>;
  using QueryTypePmrMap = ankerl::unordered_dense::pmr::map<std::string_view, KvQueryType>;


  template<class Alloc>
  auto createLocalHandlers (Alloc& alloc)
  {
    // initialise with 1 bucket and pmr allocator
    HandlerPmrMap h (
    {
      {KvQueryType::KvSave,         Handler{std::bind_front(&KvHandler::save,         std::ref(*this))}},
      {KvQueryType::KvLoad,         Handler{std::bind_front(&KvHandler::load,         std::ref(*this))}},
    }, 1, alloc);
    
    return h;
  }

  struct ValidateExecute
  {
    std::function<RequestStatus(const std::string_view, const std::string_view, const njson&)> validate;
    std::function<Response(CacheMap&, const njson&)> execute;
    std::tuple<std::string_view, std::string_view> reqrsp;
  };


  const std::map<KvQueryType, ValidateExecute> ExecutorHandlers = 
  {
    {
      KvQueryType::KvSet,      ValidateExecute
                              {
                                .validate = validateSet,
                                .execute = KvExecutor<false>::set,
                                .reqrsp = {SetReq, SetRsp}
                              }
    },
    {
      KvQueryType::KvGet,      ValidateExecute
                              {
                                .validate = validateGet,
                                .execute = KvExecutor<false>::get,
                                .reqrsp = {GetReq, GetRsp}
                              }
    },
    {
      KvQueryType::KvAdd,      ValidateExecute
                              {
                                .validate = validateAdd,
                                .execute = KvExecutor<false>::add,
                                .reqrsp = {AddReq, AddRsp}
                              }
    },
    {
      KvQueryType::KvRemove,  ValidateExecute
                              {
                                .validate = validateRemove,
                                .execute = KvExecutor<false>::remove,
                                .reqrsp = {RmvReq, RmvRsp}
                              }
    },
    {
      KvQueryType::KvClear,   ValidateExecute
                              {
                                .validate = [](const std::string_view, const std::string_view, const njson&){return RequestStatus::Ok;},
                                .execute = KvExecutor<false>::clear,
                                .reqrsp = {ClearReq, ClearRsp}
                              }
    },
    {
      KvQueryType::KvCount,   ValidateExecute
                              {
                                .validate = [](const std::string_view, const std::string_view, const njson&){return RequestStatus::Ok;},
                                .execute = KvExecutor<false>::count,
                                .reqrsp = {CountReq, CountRsp}
                              }
    },
    {
      KvQueryType::KvContains,  ValidateExecute
                                {
                                  .validate = validateContains,
                                  .execute = KvExecutor<false>::contains,
                                  .reqrsp = {ContainsReq, ContainsRsp}
                                }
    },
    {
      KvQueryType::KvKeys,      ValidateExecute
                                {
                                  .validate = [](const std::string_view, const std::string_view, const njson&){return RequestStatus::Ok;},
                                  .execute = KvExecutor<false>::keys,
                                  .reqrsp = {KeysReq, KeysRsp}
                                }
    },
    {
      KvQueryType::KvClearSet,  ValidateExecute
                                {
                                  .validate = validateClearSet,
                                  .execute = KvExecutor<false>::clearSet,
                                  .reqrsp = {ClearSetReq, ClearSetRsp}
                                }
    }
  };


  template<class Alloc>
  auto createQueryTypeNameMap (Alloc& alloc)
  {
    QueryTypePmrMap map ( 
    {  
      {SetReq,          KvQueryType::KvSet},
      {GetReq,          KvQueryType::KvGet},
      {AddReq,          KvQueryType::KvAdd},      
      {RmvReq,          KvQueryType::KvRemove},
      {ClearReq,        KvQueryType::KvClear},
      {CountReq,        KvQueryType::KvCount},
      {ContainsReq,     KvQueryType::KvContains},
      {KeysReq,         KvQueryType::KvKeys},
      {ClearSetReq,     KvQueryType::KvClearSet},
      {SaveReq,         KvQueryType::KvSave},
      {LoadReq,         KvQueryType::KvLoad}
    }, 1, alloc); 

    return map;
  }


public:
   

  Response handle(const std::string_view& command, njson& request)
  {      
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> handlerPmrResource; // TODO buffer size
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> queryTypeNamePmrResource; // TODO buffer size
    static const HandlerPmrMap LocalHandlers{createLocalHandlers(handlerPmrResource.getAlloc())}; 
    static const QueryTypePmrMap QueryNameToType{createQueryTypeNameMap(queryTypeNamePmrResource.getAlloc())};
    

    if (const auto itType = QueryNameToType.find(command) ; itType == QueryNameToType.cend())
      return Response {.rsp = createErrorResponse(RequestStatus::CommandNotExist)};
    else if (const auto localHandlerIt = LocalHandlers.find(itType->second) ; localHandlerIt != LocalHandlers.cend())
    {
      // not sent to an executor function
      try
      {
        auto& handler = localHandlerIt->second;
        return handler(request);
      }
      catch (const std::exception& kex)
      {
        PLOGF << kex.what() ;
      }

      return Response {.rsp = createErrorResponse(RequestStatus::Unknown)};
    }
    else if  (const auto execHandlerIt = ExecutorHandlers.find(itType->second) ; execHandlerIt != ExecutorHandlers.cend())
    {
      // validate and execute
      const auto& [reqName, rspName] = execHandlerIt->second.reqrsp;
      return validateAndExecute(execHandlerIt, request, reqName, rspName);
    }
    else
      return Response {.rsp = createErrorResponse(RequestStatus::CommandNotExist)};
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
    const auto& name = cmd.at("name").as_string();
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
        response = KvExecutor<false>::saveKv(m_map, dataPath, name);
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
  }
  

  njson doLoad (const std::string& loadName, const fs::path& dataSetsRoot)
  {
    PLOGI << "Loading from " << dataSetsRoot;
    
    Response response = KvExecutor<false>::loadKv (loadName, m_map, dataSetsRoot);

    PLOGI << "Loading complete";

    return response.rsp;
  }


private:
  const Settings& m_settings;
  CacheMap m_map;
};

}
}

#endif
