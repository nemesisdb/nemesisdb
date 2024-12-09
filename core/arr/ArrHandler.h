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


template<typename T>
consteval JsonType toJsonType ()
{
  if constexpr (std::is_same_v<T, std::int64_t>)
    return JsonInt;
  else if constexpr (std::is_same_v<T, std::string>)
    return JsonString;
  else if constexpr (std::is_same_v<T, njson>)
    return JsonObject;
  else
    static_assert(false, "Array does support this type");
}



template<typename T, typename Cmds>
class ArrHandler
{
  using ArrayT = Array<T>;
  using Arrays = ankerl::unordered_dense::map<std::string, Array<T>>; // array name->arrays container
  using Iterator = ankerl::unordered_dense::map<std::string, Array<T>>::iterator;
  using ConstIterator = ankerl::unordered_dense::map<std::string, Array<T>>::const_iterator;


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
      {ArrQueryType::Create,          Handler{std::bind_front(&ArrHandler<T, Cmds>::createArray,        std::ref(*this))}},
      {ArrQueryType::Delete,          Handler{std::bind_front(&ArrHandler<T, Cmds>::deleteArray,        std::ref(*this))}},
      {ArrQueryType::DeleteAll,       Handler{std::bind_front(&ArrHandler<T, Cmds>::deleteAll,          std::ref(*this))}},
      {ArrQueryType::Set,             Handler{std::bind_front(&ArrHandler<T, Cmds>::set,                std::ref(*this))}},
      {ArrQueryType::SetRng,          Handler{std::bind_front(&ArrHandler<T, Cmds>::setRange,           std::ref(*this))}},
      {ArrQueryType::Get,             Handler{std::bind_front(&ArrHandler<T, Cmds>::get,                std::ref(*this))}},
      {ArrQueryType::GetRng,          Handler{std::bind_front(&ArrHandler<T, Cmds>::getRange,           std::ref(*this))}},
      {ArrQueryType::Len,             Handler{std::bind_front(&ArrHandler<T, Cmds>::length,             std::ref(*this))}},
      {ArrQueryType::Swap,            Handler{std::bind_front(&ArrHandler<T, Cmds>::swap,               std::ref(*this))}},
      {ArrQueryType::Exist,           Handler{std::bind_front(&ArrHandler<T, Cmds>::exist,              std::ref(*this))}},
      {ArrQueryType::Clear,           Handler{std::bind_front(&ArrHandler<T, Cmds>::clear,              std::ref(*this))}},
    }, 1, alloc);
    
    return h;
  }


  template<class Alloc>
  auto createQueryTypeNameMap (Alloc& alloc)
  {
    QueryTypePmrMap map ( 
    {  
      {Cmds::CreateReq,       ArrQueryType::Create},
      {Cmds::DeleteReq,       ArrQueryType::Delete},
      {Cmds::DeleteAllReq,    ArrQueryType::DeleteAll},
      {Cmds::SetReq,          ArrQueryType::Set},
      {Cmds::SetRngReq,       ArrQueryType::SetRng},
      {Cmds::GetReq,          ArrQueryType::Get},
      {Cmds::GetRngReq,       ArrQueryType::GetRng},
      {Cmds::LenReq,          ArrQueryType::Len},
      {Cmds::SwapReq,         ArrQueryType::Swap},
      {Cmds::ExistReq,        ArrQueryType::Exist},
      {Cmds::ClearReq,        ArrQueryType::Clear},
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
    static constexpr auto ReqName = Cmds::CreateReq.data();
    static constexpr auto RspName = Cmds::CreateRsp.data();

    const auto& reqBody = request.at(ReqName);

    if (auto [valid, rsp] = validateCreate<Cmds>(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else if (const auto& name = reqBody.at("name").as_string(); arrayExist(name))
      return Response{.rsp = createErrorResponseNoTkn(RspName, RequestStatus::Duplicate)};
    else if (const std::size_t size = reqBody.at("len").template as<std::size_t>(); !ArrayT::isRequestedSizeValid(size))
      return Response{.rsp = createErrorResponseNoTkn(RspName, RequestStatus::ValueSize)};
    else
    {
      Response response;
      response.rsp = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

      try
      {
        const std::size_t size = reqBody.at("len").template as<std::size_t>();
        [[maybe_unused]] const auto [it, emplaced] = m_arrays.try_emplace(name, ArrayT{size});
        // already checked the array name does not exist, so can ignore try_emplace() return val
      }
      catch(const std::exception& e)
      {
        PLOGE << __PRETTY_FUNCTION__ << e.what();
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
      }

      return response;
    }
  }


  ndb_always_inline Response deleteArray(njson& request)
  {
    return Response{};
    /*
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
    */
  }

  
  ndb_always_inline Response deleteAll(njson& request)
  {
    return Response{};
    /*
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
    */
  }


  ndb_always_inline Response set(njson& request)
  {
    static const auto itemType = toJsonType<T>();

    if (auto [valid, rsp] = validateSet<Cmds>(request, itemType) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& body = request.at(Cmds::SetReq);
      if (auto [exist, it] = getArray(body) ; !exist)
        return Response{.rsp = createErrorResponseNoTkn(Cmds::SetRsp, RequestStatus::NotExist)};
      else
        return ArrayExecutor<ArrayT, Cmds>::set(it->second, body);
    }
  }


  ndb_always_inline Response setRange(njson& request)
  {
    return Response{};
    /*
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
    */
  }


  ndb_always_inline Response get(njson& request)
  {
    if (auto [valid, rsp] = validateGet<Cmds>(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& body = request.at(Cmds::GetReq);
      if (auto [exist, it] = getArray(body) ; !exist)
        return Response{.rsp = createErrorResponseNoTkn(Cmds::GetRsp, RequestStatus::NotExist)};
      else
        return ArrayExecutor<ArrayT, Cmds>::get(it->second, body);
    }
  }


  ndb_always_inline Response getRange(njson& request)
  {
    // TODO 'stop' optional
    
    return Response{};
    /*
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
    */
  }


  ndb_always_inline Response length(njson& request)
  {
    return Response{};
    /*
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
    */
  }


  ndb_always_inline Response swap(njson& request)
  {
    return Response{};
    /*
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
    */
  }


  ndb_always_inline Response exist(njson& request)
  {
    return Response{};
    /*
    if (auto [valid, err] = validateExist(request) ; !valid)
      return Response{.rsp = std::move(err)};
    else
    {
      static const njson Prepared = njson{jsoncons::json_object_arg, {{ExistRsp, njson::object()}}};

      const auto& name = request.at(ExistReq).at("name").as_string();
      const auto status = toUnderlying(arrayExist(name) ? RequestStatus::Ok : RequestStatus::NotExist);

      njson rsp {Prepared};
      rsp[ExistRsp]["st"] = status;

      return Response { .rsp = std::move(rsp)};
    }
    */
  }


  ndb_always_inline Response clear(njson& request)
  {
    return Response{};
    /*
    if (auto [valid, rsp] = validateClear(request) ; !valid)
      return Response{.rsp = std::move(rsp)};
    else
    {
      const auto& body = request.at(ClearReq);
      if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
        return Response{.rsp = createErrorResponseNoTkn(ClearRsp, RequestStatus::NotExist)};
      else
        return ArrayExecutor::clear(it->second, body);
    }
    */
  }

  

private:

  std::tuple<bool, Iterator> getArray (const std::string& name, const njson& cmd)
  {
    const auto it = m_arrays.find(name);
    return {it != m_arrays.end(), it};
  }

  std::tuple<bool, Iterator> getArray (const njson& cmd)
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
  Cmds m_cmds;
};

}
}

#endif
