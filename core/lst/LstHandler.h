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
    using HandlerPmrMap = ankerl::unordered_dense::pmr::map<LstQueryType, Handler>;
    using QueryTypePmrMap = ankerl::unordered_dense::pmr::map<std::string_view, LstQueryType>;


  public:

    // these are handled by this class, rather than by the executor
    template<class Alloc>
    auto createLocalHandlers (Alloc& alloc)
    {
      // initialise with 1 bucket and pmr allocator
      HandlerPmrMap h (
      {
        {LstQueryType::Create,          Handler{std::bind_front(&LstHandler<T, Cmds>::create,        std::ref(*this))}},
        {LstQueryType::Delete,          Handler{std::bind_front(&LstHandler<T, Cmds>::deleteList,    std::ref(*this))}},
        {LstQueryType::DeleteAll,       Handler{std::bind_front(&LstHandler<T, Cmds>::deleteAll,     std::ref(*this))}},
        {LstQueryType::Exist,           Handler{std::bind_front(&LstHandler<T, Cmds>::exist,         std::ref(*this))}},
        {LstQueryType::Splice,           Handler{std::bind_front(&LstHandler<T, Cmds>::splice,        std::ref(*this))}},
      }, 1, alloc);
      
      return h;
    }


    template<class Alloc>
    auto createQueryTypeNameMap (Alloc& alloc)
    {
      QueryTypePmrMap map ( 
      {  
        {Cmds::create.req,      LstQueryType::Create},
        {Cmds::del.req,         LstQueryType::Delete},
        {Cmds::deleteAll.req,   LstQueryType::DeleteAll},
        {Cmds::exist.req,       LstQueryType::Exist},
        {Cmds::len.req,         LstQueryType::Len},
        {Cmds::add.req,         LstQueryType::Add},
        {Cmds::get.req,         LstQueryType::Get},
        {Cmds::getRng.req,      LstQueryType::GetRng},        
        {Cmds::setRng.req,      LstQueryType::SetRng},
        {Cmds::remove.req,      LstQueryType::Remove},
        {Cmds::splice.req,      LstQueryType::Splice},
      }, 1, alloc); 

      return map;
    }


    struct ValidateExecute
    {
      std::function<RequestStatus(const njson&)> validate;
      std::function<Response(ListT&, const njson&)> execute;
      std::string_view rspName;
    };


    const std::map<LstQueryType, ValidateExecute> ExecutorHandlers = 
    {
      { LstQueryType::Add,      ValidateExecute
                                {
                                  .validate = validateAdd<Cmds>,
                                  .execute = ListExecutor<ListT, Cmds>::add,
                                  .rspName = Cmds::create.rsp.data()
                                }
      },
      {
        LstQueryType::SetRng,   ValidateExecute
                                {
                                  .validate = validateSetRange<Cmds>,
                                  .execute = ListExecutor<ListT, Cmds>::setRange,
                                  .rspName = Cmds::setRng.rsp.data()
                                }
      },
      {
        LstQueryType::Get,      ValidateExecute
                                {
                                  .validate = validateGet<Cmds>,
                                  .execute = ListExecutor<ListT, Cmds>::get,
                                  .rspName = Cmds::get.rsp.data()
                                }
      },
      {
        LstQueryType::GetRng,   ValidateExecute
                                {
                                  .validate = validateGetRange<Cmds>,
                                  .execute = ListExecutor<ListT, Cmds>::getRange,
                                  .rspName = Cmds::getRng.rsp.data()
                                }
      },
      {
        LstQueryType::Remove,   ValidateExecute
                                {
                                  .validate = validateRemove<Cmds>,
                                  .execute = ListExecutor<ListT, Cmds>::remove,
                                  .rspName = Cmds::remove.rsp.data()
                                }
      },
      {
        LstQueryType::Len,      ValidateExecute
                                {
                                  .validate = validateLength<Cmds>,
                                  .execute = ListExecutor<ListT, Cmds>::length,
                                  .rspName = Cmds::len.rsp.data()
                                }
      }
    };
    
  
  public:

    Response handle(const std::string_view& reqName, njson& request)
    {      
      static PmrResource<typename HandlerPmrMap::value_type, 1024U> handlerPmrResource; // TODO buffer size
      static PmrResource<typename HandlerPmrMap::value_type, 1024U> queryTypeNamePmrResource; // TODO buffer size
      static const QueryTypePmrMap QueryNameToType{createQueryTypeNameMap(queryTypeNamePmrResource.getAlloc())};
      static const HandlerPmrMap LocalHandlers{createLocalHandlers(handlerPmrResource.getAlloc())};

      if (const auto itType = QueryNameToType.find(reqName) ; itType == QueryNameToType.cend())
      {
        return Response {.rsp = createErrorResponse(RequestStatus::CommandNotExist)};
      }
      else if (const auto handlerIt = LocalHandlers.find(itType->second) ; handlerIt != LocalHandlers.cend())
      {
        // not sent to the executor
        try
        {
          auto& handler = handlerIt->second;
          return handler(request);
        }
        catch (const std::exception& ex)
        {
          PLOGE << ex.what() ;
          return Response {.rsp = createErrorResponse(RequestStatus::Unknown)};
        }
      }
      else if (const auto handlerIt = ExecutorHandlers.find(itType->second) ; handlerIt != ExecutorHandlers.cend())
      {
        return validateAndExecute(handlerIt, request, reqName);
      }
      else
      {
        return Response {.rsp = createErrorResponse(RequestStatus::CommandNotExist)};
      }
    }


  private:

    Response validateAndExecute(const std::map<LstQueryType, ValidateExecute>::const_iterator it, const njson& request, const std::string_view reqName)
    {
      const auto& validateExecute = it->second;

      if (const auto status = validateExecute.validate(request); status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(validateExecute.rspName, status)};
      else
      {
        const auto& body = request.at(reqName);

        if (auto [exist, itNameToList] = getList(body); !exist)
          return Response{.rsp = createErrorResponse(validateExecute.rspName, RequestStatus::NotExist)};
        else
          return validateExecute.execute(itNameToList->second, body);
      }
    }


    Response create(njson& request)
    {
      static constexpr auto ReqName = Cmds::create.req.data();
      static constexpr auto RspName = Cmds::create.rsp.data();
      static const njson Prepared {jsoncons::json_object_arg, {{RspName, njson::object()}}};

      const auto& reqBody = request.at(ReqName);

      Response response;
      response.rsp = Prepared;
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

      if (const auto status = validateCreate<Cmds>(request); status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else if (const auto& name = reqBody.at("name").as_string(); listExists(name))
        return Response{.rsp = createErrorResponse(RspName, RequestStatus::Duplicate)};
      else
      {
        try
        {
          const auto& name = reqBody.at("name").as_string();
          createList(name);
        }
        catch(const std::exception& e)
        {
          PLOGE << __PRETTY_FUNCTION__ << e.what();
          response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
        }

        return response;
      }
    }

    
    Response deleteList(njson& request)
    {
      static constexpr auto ReqName = Cmds::del.req.data();
      static constexpr auto RspName = Cmds::del.rsp.data();
      static const njson Prepared {jsoncons::json_object_arg, {{RspName, njson::object()}}};

      const auto& reqBody = request.at(ReqName);

      Response response;
      response.rsp = Prepared;
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

      if (const auto status = validateDelete<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else if (const auto& name = reqBody.at("name").as_string(); !listExists(name))
        return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
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


    Response deleteAll(njson& request)
    {
      static const constexpr auto RspName = Cmds::deleteAll.rsp.data();
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


    Response exist(njson& request)
    {
      static constexpr auto ReqName = Cmds::exist.req.data();
      static constexpr auto RspName = Cmds::exist.rsp.data();
      static const njson Prepared {jsoncons::json_object_arg, {{RspName, njson::object()}}};

      if (const auto status = validateExist<Cmds>(request) ; status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else
      {
        const auto& name = request.at(ReqName).at("name").as_string();

        njson rsp {jsoncons::json_object_arg, {{RspName, njson{}}}}; 
        rsp[RspName]["st"] = toUnderlying(listExists(name) ? RequestStatus::Ok : RequestStatus::NotExist);

        return Response { .rsp = std::move(rsp)};
      }
    }

    
    Response splice(njson& request)
    {
      static constexpr auto ReqName = Cmds::splice.req.data();
      static constexpr auto RspName = Cmds::splice.rsp.data();
      static const njson Prepared {jsoncons::json_object_arg, {{RspName, njson::object()}}};

      const auto& body = request.at(ReqName);
      const auto& srcName = body.at("srcName").as_string();
      const auto& destName = body.at("destName").as_string();

      if (const auto status = validateSplice<Cmds>(request); status != RequestStatus::Ok)
        return Response{.rsp = createErrorResponse(RspName, status)};
      else if (!listExists(srcName))
        return Response{.rsp = createErrorResponse(RspName, RequestStatus::NotExist)};
      else
      {
        Response response{.rsp = Prepared};
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

        try
        {
          const Iterator itDest = createList(destName); // does nothing if dest already exists
          const auto [srcExists, itSrc] = getList(body, "srcName");
                    
          ListT& srcList = itSrc->second;
          ListT& destList = itDest->second;
          const std::size_t destPos = body.contains("destPos") ? body.at("destPos").template as<std::size_t>() : 0U;
          const auto [srcStart, srcStop, hasStop, hasRange] = rangeFromRequest(body, "srcRng");

          if (hasStop)
            destList.splice(destPos, srcList, srcStart, srcStop);
          else
            destList.splice(destPos, srcList, srcStart);
        }
        catch(const std::exception& e)
        {
          PLOGE << e.what();
          response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
        }

        return response;
      }
    }


  private:

    // helpers

    Iterator createList (const std::string& name)
    {
      #ifdef NDB_DEBUG
      const auto [it, emplaced] = m_lists.try_emplace(name, name);
      #else
      const auto [it, emplaced] = m_lists.insert({name, ListT{}});
      #endif

      PLOGD_IF(emplaced) << "Created list " << name;

      return it;
    }

    
    std::tuple<bool, Iterator> getList (const std::string& listName)
    {
      const auto it = m_lists.find(listName);
      return {it != m_lists.end(), it};
    }


    std::tuple<bool, Iterator> getList (const njson& body, const std::string_view field = "name")
    {
      const auto& name = body.at(field).as_string();
      return getList(name);
    }


    bool listExists (const std::string& name)
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
