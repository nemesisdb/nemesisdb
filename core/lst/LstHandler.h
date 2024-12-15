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
        {LstQueryType::Add,             Handler{std::bind_front(&LstHandler<T, Cmds>::add,           std::ref(*this))}},
        {LstQueryType::Get,             Handler{std::bind_front(&LstHandler<T, Cmds>::get,           std::ref(*this))}},
        {LstQueryType::GetRng,          Handler{std::bind_front(&LstHandler<T, Cmds>::getRange,      std::ref(*this))}},
        {LstQueryType::DeleteAll,       Handler{std::bind_front(&LstHandler<T, Cmds>::deleteAll,     std::ref(*this))}},
        {LstQueryType::SetRng,          Handler{std::bind_front(&LstHandler<T, Cmds>::setRange,      std::ref(*this))}},
      }, 1, alloc);
      
      return h;
    }


    template<class Alloc>
    auto createQueryTypeNameMap (Alloc& alloc)
    {
      QueryTypePmrMap map ( 
      {  
        {Cmds::CreateReq,       LstQueryType::Create},
        {Cmds::AddReq,          LstQueryType::Add},
        {Cmds::GetReq,          LstQueryType::Get},
        {Cmds::GetRngReq,       LstQueryType::GetRng},
        {Cmds::DeleteAllReq,    LstQueryType::DeleteAll},
        {Cmds::SetRngReq,       LstQueryType::SetRng},
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

      if (auto [valid, rsp] = validateCreate<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(rsp)};
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

    
    ndb_always_inline Response add(njson& request)
    {
      if (auto [valid, rsp] = validateAdd<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(rsp)};
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
      if (auto [valid, rsp] = validateSetRange<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(rsp)};
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
      if (auto [valid, rsp] = validateGet<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(rsp)};
      else
      {
        const auto& body = request.at(Cmds::GetReq);
        if (auto [exist, it] = getList(body); !exist)
          return Response{.rsp = createErrorResponseNoTkn(Cmds::GetRsp, RequestStatus::NotExist)};
        else
        {
          return ListExecutor<ListT, Cmds>::get(it->second, body);
        }
      }
    }


    ndb_always_inline Response getRange(njson& request)
    {
      if (auto [valid, rsp] = validateGetRange<Cmds>(request) ; !valid)
        return Response{.rsp = std::move(rsp)};
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
