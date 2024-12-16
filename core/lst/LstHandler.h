#ifndef NDB_CORE_LSTHANDLERS_H
#define NDB_CORE_LSTHANDLERS_H


#include <functional>
#include <tuple>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/NemesisConfig.h>
#include <core/lst/LstCommon.h>
#include <core/lst/LstCommands.h>
#include <core/lst/LstCommandValidate.h>
#include <core/lst/LstExecutor.h>
#include <core/lst/LstList.h>



namespace nemesis { namespace lst {


  using namespace nemesis::lst::cmds;


  template<typename T, typename Cmds>
  class LstHandler
  {
    using ListT = List<T>;
    using Lists = ankerl::unordered_dense::map<std::string, List<T>>; 
    using Iterator = ankerl::unordered_dense::map<std::string, List<T>>::iterator;
    using ConstIterator = ankerl::unordered_dense::map<std::string, List<T>>::const_iterator;


  public:
    LstHandler() = default;

    using HandlerPmrMap = ankerl::unordered_dense::pmr::map<LstQueryType, Handler>;
    using QueryTypePmrMap = ankerl::unordered_dense::pmr::map<std::string_view, LstQueryType>;


    template<class Alloc>
    auto createHandlers (Alloc& alloc)
    {
      // initialise with 1 bucket and pmr allocator
      HandlerPmrMap h (
      {
        {LstQueryType::Create,          Handler{std::bind_front(&LstHandler<T, Cmds>::create,        std::ref(*this))}},
        {LstQueryType::Delete,          Handler{std::bind_front(&LstHandler<T, Cmds>::deleteList,    std::ref(*this))}},
        {LstQueryType::DeleteAll,       Handler{std::bind_front(&LstHandler<T, Cmds>::deleteAll,     std::ref(*this))}},
        {LstQueryType::Exist,           Handler{std::bind_front(&LstHandler<T, Cmds>::exist,         std::ref(*this))}},
        {LstQueryType::Len,             Handler{std::bind_front(&LstHandler<T, Cmds>::length,        std::ref(*this))}},
        {LstQueryType::Add,             Handler{std::bind_front(&LstHandler<T, Cmds>::add,           std::ref(*this))}},
        {LstQueryType::Get,             Handler{std::bind_front(&LstHandler<T, Cmds>::get,           std::ref(*this))}},
        {LstQueryType::GetRng,          Handler{std::bind_front(&LstHandler<T, Cmds>::getRange,      std::ref(*this))}},        
        {LstQueryType::SetRng,          Handler{std::bind_front(&LstHandler<T, Cmds>::setRange,      std::ref(*this))}},
        {LstQueryType::Remove,          Handler{std::bind_front(&LstHandler<T, Cmds>::remove,        std::ref(*this))}},
      }, 1, alloc);
      
      return h;
    }


    template<class Alloc>
    auto createQueryTypeNameMap (Alloc& alloc)
    {
      QueryTypePmrMap map ( 
      {  
        {Cmds::CreateReq,       LstQueryType::Create},
        {Cmds::DeleteReq,       LstQueryType::Delete},
        {Cmds::DeleteAllReq,    LstQueryType::DeleteAll},
        {Cmds::ExistReq,        LstQueryType::Exist},
        {Cmds::LenReq,          LstQueryType::Len},
        {Cmds::AddReq,          LstQueryType::Add},
        {Cmds::GetReq,          LstQueryType::Get},
        {Cmds::GetRngReq,       LstQueryType::GetRng},        
        {Cmds::SetRngReq,       LstQueryType::SetRng},
        {Cmds::RemoveReq,       LstQueryType::Remove},
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
    ndb_always_inline Response create(njson& request)
    {
      static constexpr auto ReqName = Cmds::CreateReq.data();
      static constexpr auto RspName = Cmds::CreateRsp.data();
      static const njson Prepared {jsoncons::json_object_arg, {{RspName, njson::object()}}};

      const auto& reqBody = request.at(ReqName);

      Response response;
      response.rsp = Prepared;
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

      if (auto [valid, err] = validateCreate<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(err)};
      else if (const auto& name = reqBody.at("name").as_string(); listExist(name))
        return Response{.rsp = createErrorResponseNoTkn(RspName, RequestStatus::Duplicate)};
      else
      {
        try
        {
          const auto& name = reqBody.at("name").as_string();

          [[maybe_unused]] const auto [it, emplaced] = m_lists.try_emplace(name, ListT{});
        }
        catch(const std::exception& e)
        {
          PLOGE << __PRETTY_FUNCTION__ << e.what();
          response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
        }

        return response;
      }
    }

    
    ndb_always_inline Response deleteList(njson& request)
    {
      static constexpr auto ReqName = Cmds::DeleteReq.data();
      static constexpr auto RspName = Cmds::DeleteRsp.data();
      static const njson Prepared {jsoncons::json_object_arg, {{RspName, njson::object()}}};

      const auto& reqBody = request.at(ReqName);

      Response response;
      response.rsp = Prepared;
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

      if (auto [valid, err] = validateDelete<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(err)};
      else if (const auto& name = reqBody.at("name").as_string(); !listExist(name))
        return Response{.rsp = createErrorResponseNoTkn(RspName, RequestStatus::NotExist)};
      else
      {
        try
        {
          m_lists.erase(name);
        }
        catch(const std::exception& e)
        {
          PLOGE << __PRETTY_FUNCTION__ << e.what();
          response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
        }

        return response;
      }
    }


    ndb_always_inline Response exist(njson& request)
    {
      static constexpr auto ReqName = Cmds::ExistReq.data();
      static constexpr auto RspName = Cmds::ExistRsp.data();
      static const njson Prepared {jsoncons::json_object_arg, {{RspName, njson::object()}}};

      if (auto [valid, err] = validateExist<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(err)};
      else
      {
        const auto& name = request.at(ReqName).at("name").as_string();
        const auto status = toUnderlying(listExist(name) ? RequestStatus::Ok : RequestStatus::NotExist);

        njson rsp {jsoncons::json_object_arg, {{RspName, njson{}}}}; 
        rsp[RspName]["st"] = status;

        return Response { .rsp = std::move(rsp)};
      }
    }


    ndb_always_inline Response length(njson& request)
    {
      static const constexpr auto RspName = Cmds::LenRsp.data();
      static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
      
      try
      {
        if (auto [valid, err] = validateLength<Cmds>(request) ; !valid)
          return Response{.rsp = std::move(err)};
        else
        {
          const auto& body = request.at(Cmds::LenReq);
          if (auto [exist, it] = getList(body); !exist)
            return Response{.rsp = createErrorResponseNoTkn(RspName, RequestStatus::NotExist)};
          else
            return ListExecutor<ListT, Cmds>::length(it->second, body);
        }
      }
      catch(const std::exception& e)
      {
        PLOGE << e.what();
        return Response{.rsp = createErrorResponseNoTkn(RspName, RequestStatus::Unknown)};
      }
    }


    ndb_always_inline Response add(njson& request)
    {
      if (auto [valid, err] = validateAdd<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(err)};
      else
      {
        const auto& body = request.at(Cmds::AddReq);
        if (auto [exist, it] = getList(body); !exist)
          return Response{.rsp = createErrorResponseNoTkn(Cmds::AddRsp, RequestStatus::NotExist)};
        else
        {
          return ListExecutor<ListT, Cmds>::add(it->second, body);
        }
      }
    }


    ndb_always_inline Response setRange(njson& request)
    {
      if (auto [valid, err] = validateSetRange<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(err)};
      else
      {
        const auto& body = request.at(Cmds::SetRngReq);
        if (auto [exist, it] = getList(body); !exist)
          return Response{.rsp = createErrorResponseNoTkn(Cmds::SetRngRsp, RequestStatus::NotExist)};
        else
        {
          return ListExecutor<ListT, Cmds>::setRange(it->second, body);
        }
      }
    }


    ndb_always_inline Response get(njson& request)
    {
      if (auto [valid, err] = validateGet<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(err)};
      else
      {
        try
        {
          const auto& body = request.at(Cmds::GetReq);
          if (auto [exist, it] = getList(body); !exist)
            return Response{.rsp = createErrorResponseNoTkn(Cmds::GetRsp, RequestStatus::NotExist)};
          else
            return ListExecutor<ListT, Cmds>::get(it->second, body);
        }
        catch(const std::exception& e)
        {
          PLOGE << e.what();
          return Response{.rsp = createErrorResponseNoTkn(Cmds::GetRsp, RequestStatus::Unknown)};
        }
      }
    }


    ndb_always_inline Response getRange(njson& request)
    {
      if (auto [valid, err] = validateGetRange<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(err)};
      else
      {
        const auto& body = request.at(Cmds::GetRngReq);
        if (auto [exist, it] = getList(body) ; !exist)
          return Response{.rsp = createErrorResponseNoTkn(Cmds::GetRngRsp, RequestStatus::NotExist)};
        else
          return ListExecutor<ListT, Cmds>::getRange(it->second, body);
      }
      return Response{};
    }


    ndb_always_inline Response deleteAll(njson& request)
    {
      static const constexpr auto RspName = Cmds::DeleteAllRsp.data();
      static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
      
      Response response{.rsp = Prepared};
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);
      
      try
      {
        m_lists.clear();
      }
      catch(const std::exception& e)
      {
        PLOGE << e.what();
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
      }

      return response;
    }


    ndb_always_inline Response remove(njson& request)
    {
      if (auto [valid, err] = validateRemove<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(err)};
      else
      {
        const auto& body = request.at(Cmds::RemoveReq);
        if (auto [exist, it] = getList(body) ; !exist)
          return Response{.rsp = createErrorResponseNoTkn(Cmds::RemoveRsp, RequestStatus::NotExist)};
        else
          return ListExecutor<ListT, Cmds>::remove(it->second, body);
      }
      return Response{};
    }


  private:

    std::tuple<bool, Iterator> getList (const std::string& name, const njson& cmd)
    {
      const auto it = m_lists.find(name);
      return {it != m_lists.end(), it};
    }


    std::tuple<bool, Iterator> getList (const njson& cmd)
    {
      const auto& name = cmd.at("name").as_string();
      return getList(name, cmd);
    }


    bool listExist (const std::string& name)
    {
      return m_lists.contains(name);
    }


  private:
    Lists m_lists;
    Cmds m_cmds;
  };

  
  using OLstHandler = LstHandler<njson, ListCmds>;
}
}

#endif
