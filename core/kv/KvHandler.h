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
  KvHandler(const Settings& settings) : m_settings(settings)
    
  {
  }
  

private:

  struct Handler
  {
    using Handle = std::function<Response(njson&)>;

    Handler(Handle&& h) : handler(std::move(h))
    {

    }

    Handler(const Handler&) = default;
    Handler(Handler&&) = default;
    Handler& operator= (Handler&&) = default;

    Response operator()(njson& request)
    {
      return handler(request);
    }

    Handle handler;
  };


  using HandlerPmrMap = ankerl::unordered_dense::pmr::map<KvQueryType, Handler>;
  using QueryTypePmrMap = ankerl::unordered_dense::pmr::map<std::string_view, KvQueryType>;


  template<class Alloc>
  auto createHandlers (Alloc& alloc)
  {
    // initialise with 1 bucket and pmr allocator
    HandlerPmrMap h (
    {
      {KvQueryType::KvSet,          Handler{std::bind_front(&KvHandler::set,        std::ref(*this))}},      
      {KvQueryType::KvGet,          Handler{std::bind_front(&KvHandler::get,        std::ref(*this))}},
      {KvQueryType::KvAdd,          Handler{std::bind_front(&KvHandler::add,        std::ref(*this))}},      
      {KvQueryType::KvRemove,       Handler{std::bind_front(&KvHandler::remove,     std::ref(*this))}},
      {KvQueryType::KvClear,        Handler{std::bind_front(&KvHandler::clear,      std::ref(*this))}},
      {KvQueryType::KvCount,        Handler{std::bind_front(&KvHandler::count,      std::ref(*this))}},
      {KvQueryType::KvContains,     Handler{std::bind_front(&KvHandler::contains,   std::ref(*this))}},
      {KvQueryType::KvFind,         Handler{std::bind_front(&KvHandler::find,       std::ref(*this))}},
      {KvQueryType::KvUpdate,       Handler{std::bind_front(&KvHandler::update,     std::ref(*this))}},
      {KvQueryType::KvKeys,         Handler{std::bind_front(&KvHandler::keys,       std::ref(*this))}},
      {KvQueryType::KvClearSet,     Handler{std::bind_front(&KvHandler::clearSet,   std::ref(*this))}},
      {KvQueryType::KvArrayAppend,  Handler{std::bind_front(&KvHandler::arrayAppend,  std::ref(*this))}},
      {KvQueryType::KvSave,         Handler{std::bind_front(&KvHandler::save,         std::ref(*this))}},
      {KvQueryType::KvLoad,         Handler{std::bind_front(&KvHandler::load,         std::ref(*this))}},
      //{KvQueryType::KvSetQ,         Handler{std::bind_front(&KvHandler::setQ,       std::ref(*this))}},
      //{KvQueryType::KvAddQ,         Handler{std::bind_front(&KvHandler::addQ,       std::ref(*this))}}      
    }, 1, alloc);
    
    return h;
  }


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
      {FindReq,         KvQueryType::KvFind},
      {UpdateReq,       KvQueryType::KvUpdate},
      {KeysReq,         KvQueryType::KvKeys},
      {ClearSetReq,     KvQueryType::KvClearSet},
      {ArrAppendReq,    KvQueryType::KvArrayAppend},
      {SaveReq,         KvQueryType::KvSave},
      {LoadReq,         KvQueryType::KvLoad}
      //{SetQReq,         KvQueryType::KvSetQ},
      //{AddQReq,         KvQueryType::KvAddQ}
    }, 1, alloc); 

    return map;
  }


public:
   

  Response handle(const std::string_view& command, njson& request)
  {      
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> handlerPmrResource; // TODO buffer size
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> queryTypeNamePmrResource; // TODO buffer size
    static HandlerPmrMap MsgHandlers{createHandlers(handlerPmrResource.getAlloc())}; 
    static QueryTypePmrMap QueryNameToType{createQueryTypeNameMap(queryTypeNamePmrResource.getAlloc())};
    

    if (const auto itType = QueryNameToType.find(command) ; itType == QueryNameToType.cend())
      return Response {.rsp = createErrorResponse(RequestStatus::CommandNotExist)};
    else if (const auto handlerIt = MsgHandlers.find(itType->second) ; handlerIt == MsgHandlers.cend())
      return Response {.rsp = createErrorResponse(RequestStatus::CommandDisabled)};
    else
    {
      try
      {
        auto& handler = handlerIt->second;
        return handler(request);
      }
      catch (const std::exception& kex)
      {
        PLOGF << kex.what() ;
      }

      return Response {.rsp = createErrorResponse(RequestStatus::Unknown)};
    }
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
    

  CacheMap& getContainer() 
  {
    return m_map;
  }


  template<typename F>
  Response executeKvCommand(const std::string_view cmdRspName, const njson& cmd, F&& handler)
    requires(std::is_invocable_v<F, CacheMap&, njson&>)
  {
    const auto& cmdRoot = cmd.object_range().cbegin()->value();
    return callKvHandler(m_map, cmdRoot, std::forward<F>(handler));
  }


  template<typename F>
  Response callKvHandler(CacheMap& map,const njson& cmdRoot, F&& handler)
    requires( std::is_invocable_v<F, CacheMap&, njson&> &&
              std::is_same_v<Response, std::invoke_result_t<F, CacheMap&, njson&>>)
  {
    return std::invoke(handler, map, cmdRoot);
  }


  
  ndb_always_inline Response set(njson& request)
  {
    if (auto [valid, rsp] = validateSet(SetReq, SetRsp, request); !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(SetRsp, request, KvExecutor<false>::set);
  }

  
  /*
  ndb_always_inline Response setQ(njson& request)
  {
    if (auto [valid, rsp] = validateSetQ(SetQReq, SetQRsp, request); !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(SetQRsp, request, KvExecutor<false>::setQ);
  }
  */
  
  ndb_always_inline Response get(njson& request)
  {
    if (auto [valid, rsp] = validateGet(GetReq, GetRsp, request); !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(GetRsp, request, KvExecutor<false>::get);
  }

  
  ndb_always_inline Response add(njson& request)
  {
    if (auto [valid, rsp] = validateAdd(AddReq, AddRsp, request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(AddRsp, request, KvExecutor<false>::add);
  }


  /*
  ndb_always_inline Response addQ(njson& request)
  {
    if (auto [valid, rsp] = validateAddQ(AddQReq, AddQRsp, request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(AddQRsp, request, KvExecutor<false>::addQ);
  }
  */


  ndb_always_inline Response remove(njson& request)
  {
    if (auto [valid, rsp] = validateRemove(RmvReq, RmvRsp, request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(RmvRsp, request, KvExecutor<false>::remove);
  }

  
  ndb_always_inline Response clear(njson& request)
  {
    return executeKvCommand(ClearRsp, request, KvExecutor<false>::clear);
  }


  ndb_always_inline Response count(njson& request)
  {
    return executeKvCommand(CountRsp, request, KvExecutor<false>::count);
  }


  ndb_always_inline Response contains(njson& request)
  {
    if (auto [valid, rsp] = validateContains(ContainsReq, ContainsRsp, request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(ContainsRsp, request, KvExecutor<false>::contains);
  }


  ndb_always_inline Response find(njson& request)
  {
    if (auto [valid, rsp] = validateFind(FindReq, FindRsp, request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      if (!request.at(FindReq).contains("keys"))
        request.at(FindReq)["keys"] = njson::array(); // executor expects "keys"

      return executeKvCommand(FindRsp, request, KvExecutor<false>::find);
    }
  }


  ndb_always_inline Response update(njson& request)
  {
    if (auto [valid, rsp] = validateUpdate(UpdateReq, UpdateRsp, request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(UpdateRsp, request, KvExecutor<false>::update);
  }


  ndb_always_inline Response keys(njson& request)
  {
    return executeKvCommand(KeysRsp, request, KvExecutor<false>::keys);
  }


  ndb_always_inline Response clearSet(njson& request)
  {
    if (auto [valid, rsp] = validateClearSet(ClearSetReq, ClearSetRsp, request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(ClearSetRsp, request, KvExecutor<false>::clearSet);
  }
  

  ndb_always_inline Response arrayAppend(njson& request)
  {
    if (auto [valid, rsp] = validateArrayAppend(ArrAppendReq, ArrAppendRsp, request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
      return executeKvCommand(ArrAppendRsp, request, KvExecutor<false>::arrayAppend);
  }

  
  Response save(njson& request)
  {
    if (!m_settings.persistEnabled)
      return Response{.rsp = createErrorResponseNoTkn(SaveRsp, RequestStatus::CommandDisabled)};
    else
    {
      if (auto [valid, rsp] = kv::validateSave(request) ; !valid)
        return Response{.rsp = std::move(rsp)};
      else
        return doSave(SaveRsp, request.at(SaveReq));
    }
  }


  Response load(njson& request)
  {
    if (auto [valid, rsp] = kv::validateLoad(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
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
      njson rsp;
      rsp[queryRspName]["name"] = name;
      rsp[queryRspName]["st"] = toUnderlying(preparedStatus);
      return Response{.rsp = std::move(rsp)};
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
    
    Response response = KvExecutor<false>::loadKv (loadName, getContainer(), dataSetsRoot);

    PLOGI << "Loading complete";

    return response.rsp;
  }


private:
  Settings m_settings;
  CacheMap m_map;
};

}
}

#endif
