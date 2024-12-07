#ifndef NDB_CORE_ARRHANDLERS_H
#define NDB_CORE_ARRHANDLERS_H


#include <functional>
#include <tuple>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/Persistance.h>
#include <core/NemesisConfig.h>
#include <core/arr/ArrCommon.h>
#include <core/arr/ArrCommandValidate.h>
#include <core/arr/ArrExecutor.h>
#include <core/arr/ArrArray.h>



namespace nemesis { namespace arr {


using namespace nemesis::arr::cmds;


/*

*/
class ArrHandler
{
  using Arrays = ankerl::unordered_dense::map<std::string, Array>;


public:
  ArrHandler(const Settings& settings) : m_settings(settings)
  {

  }


  using HandlerPmrMap = ankerl::unordered_dense::pmr::map<ArrQueryType, Handler>;
  using QueryTypePmrMap = ankerl::unordered_dense::pmr::map<std::string_view, ArrQueryType>;


  template<class Alloc>
  auto createHandlers (Alloc& alloc)
  {
    // initialise with 1 bucket and pmr allocator
    HandlerPmrMap h (
    {
      {ArrQueryType::Create,          Handler{std::bind_front(&ArrHandler::createArray,        std::ref(*this))}},      
      {ArrQueryType::Delete,          Handler{std::bind_front(&ArrHandler::deleteArray,        std::ref(*this))}},
      {ArrQueryType::DeleteAll,       Handler{std::bind_front(&ArrHandler::deleteAll,          std::ref(*this))}},
      {ArrQueryType::Set,             Handler{std::bind_front(&ArrHandler::set,                std::ref(*this))}},
      {ArrQueryType::SetRng,          Handler{std::bind_front(&ArrHandler::setRange,           std::ref(*this))}},
      {ArrQueryType::Get,             Handler{std::bind_front(&ArrHandler::get,                std::ref(*this))}},
      {ArrQueryType::GetRng,          Handler{std::bind_front(&ArrHandler::getRange,           std::ref(*this))}},
      {ArrQueryType::Len,             Handler{std::bind_front(&ArrHandler::length,             std::ref(*this))}},
      {ArrQueryType::Swap,            Handler{std::bind_front(&ArrHandler::swap,               std::ref(*this))}},
    }, 1, alloc);
    
    return h;
  }


  template<class Alloc>
  auto createQueryTypeNameMap (Alloc& alloc)
  {
    QueryTypePmrMap map ( 
    {  
      {CreateReq,           ArrQueryType::Create},
      {DeleteReq,           ArrQueryType::Delete},
      {DeleteAllReq,        ArrQueryType::DeleteAll},
      {SetReq,              ArrQueryType::Set},
      {SetRngReq,           ArrQueryType::SetRng},
      {GetReq,              ArrQueryType::Get},
      {GetRngReq,           ArrQueryType::GetRng},
      {LenReq,              ArrQueryType::Len},
      {SwapReq,             ArrQueryType::Swap},
    }, 1, alloc); 

    return map;
  }


public:
   

  Response handle(const std::string_view& command, njson& request)
  {      
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> handlerPmrResource; // TODO buffer size
    static PmrResource<typename HandlerPmrMap::value_type, 1024U> queryTypeNamePmrResource; // TODO buffer size
    static const HandlerPmrMap MsgHandlers{createHandlers(handlerPmrResource.getAlloc())}; 
    static const QueryTypePmrMap QueryNameToType{createQueryTypeNameMap(queryTypeNamePmrResource.getAlloc())};
    

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


private:
  ndb_always_inline Response createArray(njson& request)
  {
    if (auto [valid, rsp] = validateCreate(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else if (const auto& name = request.at(CreateReq).at("name").as_string(); arrayExist(name))
      return Response{.rsp = createErrorResponseNoTkn(CreateRsp, RequestStatus::Duplicate)};
    else if (const auto size = request.at(CreateReq).at("len").as<std::size_t>(); !Array::isRequestedSizeValid(size))
      return Response{.rsp = createErrorResponseNoTkn(CreateRsp, RequestStatus::ValueSize)};
    else
    {
      Response response;
      response.rsp = njson{jsoncons::json_object_arg, {{CreateRsp, njson::object()}}};
      response.rsp[CreateRsp]["st"] = toUnderlying(RequestStatus::Ok);

      try
      {
        const std::size_t size = request.at(CreateReq).at("len").as<std::size_t>();
        [[maybe_unused]] const auto [it, emplaced] = m_arrays.try_emplace(name, Array{size});
        // already checked the array name does not exist, so can ignore try_emplace() return val
      }
      catch(const std::exception& e)
      {
        PLOGE << __PRETTY_FUNCTION__ << e.what();
        response.rsp[CreateRsp]["st"] = toUnderlying(RequestStatus::Unknown);
      }

      return response;
    }
  }


  ndb_always_inline Response deleteArray(njson& request)
  {
    if (auto [valid, rsp] = validateDelete(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      Response response;
      response.rsp = njson{jsoncons::json_object_arg, {{DeleteRsp, njson::object()}}};
      response.rsp[DeleteRsp]["st"] = toUnderlying(RequestStatus::Ok);

      try
      {
        m_arrays.erase(request.at(DeleteReq).at("name").as_string());
      }
      catch(const std::exception& e)
      {
        PLOGE << __PRETTY_FUNCTION__ << e.what();
        response.rsp[DeleteRsp]["st"] = toUnderlying(RequestStatus::Unknown);
      }

      return response;
    }
  }

  
  ndb_always_inline Response deleteAll(njson& request)
  {
    Response response;
    response.rsp = njson{jsoncons::json_object_arg, {{DeleteAllRsp, njson::object()}}};
    response.rsp[DeleteAllRsp]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      m_arrays.clear();
      //m_arrays.replace() // TODO look at replace()
    }
    catch(const std::exception& e)
    {
      PLOGE << __PRETTY_FUNCTION__ << e.what();
      response.rsp[DeleteAllRsp]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  ndb_always_inline Response set(njson& request)
  {
    if (auto [valid, rsp] = validateSet(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& body = request.at(SetReq);
      if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
        return Response{.rsp = createErrorResponseNoTkn(SetRsp, RequestStatus::NotExist)};
      else
        return ArrayExecutor::set(it->second, body);
    }
  }


  ndb_always_inline Response setRange(njson& request)
  {
    if (auto [valid, rsp] = validateSetRange(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& body = request.at(SetRngReq);
      if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
        return Response{.rsp = createErrorResponseNoTkn(SetRngRsp, RequestStatus::NotExist)};
      else
        return ArrayExecutor::setRange(it->second, body);
    }
  }


  ndb_always_inline Response get(njson& request)
  {
    if (auto [valid, rsp] = validateGet(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& body = request.at(GetReq);
      if (auto [exist, it] = getArray(body) ; !exist)
        return Response{.rsp = createErrorResponseNoTkn(GetRsp, RequestStatus::NotExist)};
      else
        return ArrayExecutor::get(it->second, body);
    }
  }


  ndb_always_inline Response getRange(njson& request)
  {
    // TODO 'stop' optional
    
    if (auto [valid, rsp] = validateGetRange(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& body = request.at(GetRngReq);
      if (auto [exist, it] = getArray(body) ; !exist)
        return Response{.rsp = createErrorResponseNoTkn(GetRngRsp, RequestStatus::NotExist)};
      else
        return ArrayExecutor::getRange(it->second, body);
    }
  }


  ndb_always_inline Response length(njson& request)
  {
    if (auto [valid, rsp] = validateLength(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& body = request.at(LenReq);
      if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
        return Response{.rsp = createErrorResponseNoTkn(LenRsp, RequestStatus::NotExist)};
      else
        return ArrayExecutor::length(it->second, body);
    }
  }


  ndb_always_inline Response swap(njson& request)
  {
    if (auto [valid, rsp] = validateSwap(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& body = request.at(SwapReq);
      if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
        return Response{.rsp = createErrorResponseNoTkn(SwapRsp, RequestStatus::NotExist)};
      else
        return ArrayExecutor::swap(it->second, body);
    }
  }

private:
  
  std::tuple<bool, Arrays::iterator> getArray (const std::string& name, const njson& cmd)
  {
    const auto it = m_arrays.find(name);
    return {it != m_arrays.cend(), it};
  }

  std::tuple<bool, Arrays::iterator> getArray (const njson& cmd)
  {
    const auto& name = cmd.at("name").as_string();
    return getArray(name, cmd);
  }

  bool arrayExist (const std::string& name)
  {
    return m_arrays.contains(name);
  }


private:
  Settings m_settings;
  Arrays m_arrays;
};

}
}

#endif
