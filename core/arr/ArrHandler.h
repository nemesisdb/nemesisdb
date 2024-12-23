#ifndef NDB_CORE_ARRHANDLERS_H
#define NDB_CORE_ARRHANDLERS_H


#include <functional>
#include <tuple>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/NemesisConfig.h>
#include <core/arr/ArrCommon.h>
#include <core/arr/ArrCommandValidate.h>
#include <core/arr/ArrExecutor.h>
#include <core/arr/ArrArray.h>



namespace nemesis { namespace arr {


  using namespace nemesis::arr::cmds;


  template<typename T, typename Cmds>
  class ArrHandler
  {
    using ArrayT = Array<T, Cmds::IsSorted>;
    using Arrays = ankerl::unordered_dense::map<std::string, Array<T, Cmds::IsSorted>>; // array name->arrays container
    using Iterator = ankerl::unordered_dense::map<std::string, Array<T, Cmds::IsSorted>>::iterator;
    using ConstIterator = ankerl::unordered_dense::map<std::string, Array<T, Cmds::IsSorted>>::const_iterator;


  public:
    ArrHandler() = default;

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
        {ArrQueryType::Used,            Handler{std::bind_front(&ArrHandler<T, Cmds>::used,               std::ref(*this))}},
        {ArrQueryType::Exist,           Handler{std::bind_front(&ArrHandler<T, Cmds>::exist,              std::ref(*this))}},
        {ArrQueryType::Clear,           Handler{std::bind_front(&ArrHandler<T, Cmds>::clear,              std::ref(*this))}},
      }, 1, alloc);
      
      
      if constexpr (!Cmds::IsSorted)
      {
        h.emplace(ArrQueryType::Swap, Handler{std::bind_front(&ArrHandler<T, Cmds>::swap, std::ref(*this))});        
      }
      else
      {
        h.emplace(ArrQueryType::Intersect,  Handler{std::bind_front(&ArrHandler<T, Cmds>::intersect,  std::ref(*this))});
        h.emplace(ArrQueryType::Min,        Handler{std::bind_front(&ArrHandler<T, Cmds>::min,        std::ref(*this))});
        h.emplace(ArrQueryType::Max,        Handler{std::bind_front(&ArrHandler<T, Cmds>::max,        std::ref(*this))});
      }
      
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
        {Cmds::UsedReq,         ArrQueryType::Used},        
        {Cmds::ExistReq,        ArrQueryType::Exist},
        {Cmds::ClearReq,        ArrQueryType::Clear},
        {Cmds::SwapReq,         ArrQueryType::Swap},
        {Cmds::IntersectReq,    ArrQueryType::Intersect},
        {Cmds::MinReq,          ArrQueryType::Min},
        {Cmds::MaxReq,          ArrQueryType::Max},
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

      if (const auto status = validateCreate<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else if (const auto& name = reqBody.at("name").as_string(); arrayExist(name))
        return Response{.rsp = createErrorResponse(RspName, RequestStatus::Duplicate)};
      else if (const std::size_t size = reqBody.at("len").template as<std::size_t>(); !ArrayT::isRequestedSizeValid(size))
        return Response{.rsp = createErrorResponse(RspName, RequestStatus::Bounds)};
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
      static constexpr auto ReqName = Cmds::DeleteReq.data();
      static constexpr auto RspName = Cmds::DeleteRsp.data();

      if (const auto status = validateDelete<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        Response response;
        response.rsp = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

        try
        {
          m_arrays.erase(request.at(ReqName).at("name").as_string());
        }
        catch(const std::exception& e)
        {
          PLOGE << __PRETTY_FUNCTION__ << e.what();
          response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
        }

        return response;
      }
    }

    
    ndb_always_inline Response deleteAll(njson& request)
    {
      Response response;
      response.rsp = njson{jsoncons::json_object_arg, {{Cmds::DeleteAllRsp.data(), njson::object()}}};
      response.rsp[Cmds::DeleteAllRsp]["st"] = toUnderlying(RequestStatus::Ok);

      try
      {
        m_arrays.clear();
        //m_arrays.replace() // TODO look at replace()
      }
      catch(const std::exception& e)
      {
        PLOGE << __PRETTY_FUNCTION__ << e.what();
        response.rsp[Cmds::DeleteAllRsp]["st"] = toUnderlying(RequestStatus::Unknown);
      }

      return response;
    }


    ndb_always_inline Response set(njson& request)
    {
      static constexpr auto RspName = Cmds::SetRsp.data();

      if (const auto status = validateSet<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        const auto& body = request.at(Cmds::SetReq);
        if (auto [exist, it] = getArray(body) ; !exist)
          return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
        else
          return ArrayExecutor<ArrayT, Cmds>::set(it->second, body);
      }
    }


    ndb_always_inline Response setRange(njson& request)
    {
      static constexpr auto RspName = Cmds::SetRngRsp.data();

      if (const auto status = validateSetRange<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        auto& body = request.at(Cmds::SetRngReq);
        if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
          return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
        else
          return ArrayExecutor<ArrayT, Cmds>::setRange(it->second, body);
      }
    }


    ndb_always_inline Response get(njson& request)
    {
      static constexpr auto RspName = Cmds::GetRsp.data();

      if (const auto status = validateGet<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        const auto& body = request.at(Cmds::GetReq);
        if (auto [exist, it] = getArray(body) ; !exist)
          return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
        else
          return ArrayExecutor<ArrayT, Cmds>::get(it->second, body);
      }
    }


    ndb_always_inline Response getRange(njson& request)
    {
      static constexpr auto RspName = Cmds::GetRngRsp.data();

      if (const auto status = validateGetRange<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        const auto& body = request.at(Cmds::GetRngReq);
        if (auto [exist, it] = getArray(body) ; !exist)
          return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
        else
          return ArrayExecutor<ArrayT, Cmds>::getRange(it->second, body);
      }
    }


    // NOTE: length is actually capacity 
    ndb_always_inline Response length(njson& request)
    {    
      static constexpr auto RspName = Cmds::LenRsp.data();

      if (const auto status = validateLength<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        const auto& body = request.at(Cmds::LenReq);
        if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
          return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
        else
          return ArrayExecutor<ArrayT, Cmds>::length(it->second, body);
      }
    }


    ndb_always_inline Response used(njson& request)
    {    
      static constexpr auto RspName = Cmds::UsedRsp.data();

      if (const auto status = validateUsed<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        const auto& body = request.at(Cmds::UsedReq);
        if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
          return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
        else
          return ArrayExecutor<ArrayT, Cmds>::used(it->second, body);
      }
    }


    ndb_always_inline Response exist(njson& request)
    {
      static constexpr auto ReqName = Cmds::ExistReq.data();
      static constexpr auto RspName = Cmds::ExistRsp.data();

      if (const auto status = validateExist<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};

        const auto& name = request.at(ReqName).at("name").as_string();

        njson rsp {jsoncons::json_object_arg, {{RspName, njson{}}}}; 
        rsp[RspName]["st"] = toUnderlying(arrayExist(name) ? RequestStatus::Ok : RequestStatus::NotExist);;
        
        return Response { .rsp = std::move(rsp)};
      }
    }


    ndb_always_inline Response clear(njson& request)
    {
      static constexpr auto RspName = Cmds::ClearRsp.data();

      if (const auto status = validateClear<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        const auto& body = request.at(Cmds::ClearReq);
        if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
          return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
        else
          return ArrayExecutor<ArrayT, Cmds>::clear(it->second, body);
      }
    }

    
    ndb_always_inline Response intersect(njson& request) requires (Cmds::IsSorted)
    {
      static constexpr auto RspName = Cmds::IntersectRsp.data();

      if (const auto status = validateIntersect<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        const auto& body = request.at(Cmds::IntersectReq);
        const auto& srcA = body.at("srcA").as_string();
        const auto& srcB = body.at("srcB").as_string();

        if (srcA == srcB)
          return Response{.rsp = createErrorResponse(RspName, RequestStatus::Duplicate)};
        else
        {
          const auto [arrAExist, arrA] = getArray(srcA, body);
          const auto [arrBExist, arrB] = getArray(srcB, body);

          if (!(arrAExist && arrBExist))
            return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
          else
            return ArrayExecutor<ArrayT, Cmds>::intersect(arrA->second, arrB->second);
        }
      }
    }


    ndb_always_inline Response swap(njson& request) requires(!Cmds::IsSorted)
    {
      static constexpr auto RspName = Cmds::SwapRsp.data();

      if (const auto status = validateSwap<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        const auto& body = request.at(Cmds::SwapReq);
        if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
          return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
        else
          return ArrayExecutor<ArrayT, Cmds>::swap(it->second, body);
      }
    }


    ndb_always_inline Response min(njson& request) requires(Cmds::IsSorted)
    {
      static constexpr auto RspName = Cmds::MinRsp.data();

      if (const auto status = validateMin<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        const auto& body = request.at(Cmds::MinReq);
        if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
          return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
        else
          return ArrayExecutor<ArrayT, Cmds>::min(it->second, body);
      }
    }   

    
    ndb_always_inline Response max(njson& request) requires(Cmds::IsSorted)
    {
      static constexpr auto RspName = Cmds::MaxRsp.data();

      if (const auto status = validateMax<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        const auto& body = request.at(Cmds::MaxReq);
        if (auto [exist, it] = getArray(body.at("name").as_string(), body) ; !exist)
          return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
        else
          return ArrayExecutor<ArrayT, Cmds>::max(it->second, body);
      }
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
    Arrays m_arrays;
    Cmds m_cmds;
  };

  
  using OArrHandler = ArrHandler<njson, OArrCmds>;
  using IntArrHandler = ArrHandler<std::int64_t, IntArrCmds>;
  using StrArrHandler = ArrHandler<std::string, StrArrCmds>;
  
  using SortedIntArrHandler = ArrHandler<std::int64_t, SortedIntArrCmds>;
  using SortedStrArrHandler = ArrHandler<std::string, SortedStrArrCmds>;
}
}

#endif
