#ifndef NDB_CORE_ARRCMDVALIDATE_H
#define NDB_CORE_ARRCMDVALIDATE_H

#include <tuple>
#include <string_view>
#include <core/NemesisCommon.h>
#include <core/arr/ArrCommands.h>
#include <core/arr/ArrCommon.h>


namespace nemesis { namespace arr {

  using namespace nemesis::arr::cmds;

  template<typename Cmds>
  RequestStatus validateCreate (const njson& request)
  {
    return isValid(Cmds::CreateRsp, request.at(Cmds::CreateReq), { {Param::required("name", JsonString)},
                                                                   {Param::required("len", JsonUInt)}});
  }


  template<typename Cmds>
  RequestStatus validateDelete (const njson& req)
  {
    return isValid(Cmds::DeleteRsp, req.at(Cmds::DeleteReq), {{Param::required("name", JsonString)}});
  }


  template<typename Cmds>
  RequestStatus validateSet (const njson& req)
  {
    ValidateParams params;

    // if container can be sorted, then request cannot set a position (since it may change after sorting)
    if constexpr (Cmds::IsSorted)
      params = {{Param::required("name", JsonString)}, {Param::variable("item")} };
    else
      params = {{Param::required("name", JsonString)}, {Param::optional("pos",  JsonUInt)}, {Param::variable("item")} };


    return isArrayCmdValid<Cmds>(Cmds::SetRsp, req.at(Cmds::SetReq), params);
  }


  template<typename Cmds>
  RequestStatus validateSetRange (const njson& req)
  {
    auto itemsValid = [](const njson& body) -> RequestStatus
    {
      for(const auto& item : body.at("items").array_range())
        if (!Cmds::isTypeValid(item.type()))
          return RequestStatus::ValueTypeInvalid;
      
      return RequestStatus::Ok;
    };


    ValidateParams params;

    // if container can be sorted, then request cannot set a position (since it may change after sorting)
    if constexpr (Cmds::IsSorted)
      params = {{Param::required("name", JsonString)}, {Param::required("items", JsonArray)}};
    else
      params = {{Param::required("name", JsonString)}, {Param::required("items", JsonArray)}, {Param::optional("pos", JsonUInt)}};

    return isArrayCmdValid<Cmds>(Cmds::SetRngRsp, req.at(Cmds::SetRngReq), params, itemsValid);
  }


  template<typename Cmds>
  RequestStatus validateGet (const njson& req)
  {
    return isValid(Cmds::GetRsp, req.at(Cmds::GetReq), { {Param::required("name", JsonString)},
                                                         {Param::required("pos",  JsonUInt)}});
  }


  template<typename Cmds>
  RequestStatus validateGetRange (const njson& req)
  {
    auto checkRng = [](const njson& body) -> RequestStatus
    {
      if (const auto nDims = body.at("rng").size(); !(nDims == 1 || nDims == 2)) [[unlikely]]
        return RequestStatus::ValueSize;
      else
      {
        const auto rngArray = body.at("rng").array_range();
        const auto first = rngArray.begin();
        
        if (!first->is_uint64())
          return RequestStatus::ValueTypeInvalid;
        else if (nDims == 2)
        {
          const auto second = std::next(first);
          if (!second->is_uint64())
            return RequestStatus::ValueTypeInvalid;
          else
          {
            const auto start = first->as<std::size_t>();
            const auto stop = second->as<std::size_t>();

            if (start > stop)
              return RequestStatus::CommandSyntax;
          }
        }
      }
      
      return RequestStatus::Ok;
    };


    return isValid(Cmds::GetRngRsp, req.at(Cmds::GetRngReq), { {Param::required("name", JsonString)},
                                                               {Param::required("rng",  JsonArray)}}, checkRng);
  }


  template<typename Cmds>
  RequestStatus validateLength (const njson& req)
  {
    return isValid(Cmds::LenRsp, req.at(Cmds::LenReq.data()), { {Param::required("name", JsonString)}});
  }


  template<typename Cmds>
  RequestStatus validateUsed (const njson& req)
  {
    return isValid(Cmds::UsedRsp, req.at(Cmds::UsedReq.data()), { {Param::required("name", JsonString)}});
  }


  template<typename Cmds>
  RequestStatus validateExist (const njson& req)
  {
    return isValid(Cmds::ExistRsp, req.at(Cmds::ExistReq), { {Param::required("name", JsonString)}});
  }


  template<typename Cmds>
  RequestStatus validateClear (const njson& req)
  {
    auto checkRange = [](const njson& body) -> RequestStatus
    {
      if (const auto nRng = body.at("rng").size(); !(nRng == 2 || nRng == 1))
        return RequestStatus::CommandSyntax;
      else
      {
        const auto rngArray = body.at("rng").array_range();
        const auto first = rngArray.begin();

        if (!first->is_uint64())
            return RequestStatus::ValueTypeInvalid;

        if (nRng == 2)
        {
          const auto second = std::next(first);
          if (!(first->is_uint64() && second->is_uint64()))
            return RequestStatus::ValueTypeInvalid;
        }
      }

      return RequestStatus::Ok;
    };

    return isValid(Cmds::ClearRsp, req.at(Cmds::ClearReq), {{Param::required("name", JsonString)},
                                                            {Param::required("rng", JsonArray)}}, checkRange);
  }


  template<typename Cmds>
  RequestStatus validateIntersect (const njson& req)
  {
    const auto reqName = Cmds::IntersectReq.data();
    const auto rspName = Cmds::IntersectRsp.data();

    return isArrayCmdValid<Cmds>(rspName, req.at(reqName), {{Param::required("srcA", JsonString)},
                                                            {Param::required("srcB", JsonString)} });
  }


  template<typename Cmds>
  RequestStatus validateSwap (const njson& req)
  {
    return isValid(Cmds::SwapRsp, req.at(Cmds::SwapReq), { {Param::required("name", JsonString)},
                                                           {Param::required("posA", JsonUInt)},
                                                           {Param::required("posB", JsonUInt)}});
  }


  template<typename Cmds>
  RequestStatus validateMin (const njson& req)
  {
    return isValid(Cmds::MinRsp, req.at(Cmds::MinReq), {{Param::required("name", JsonString)},
                                                        {Param::required("n",    JsonUInt)}});
  }

  
  template<typename Cmds>
  RequestStatus validateMax (const njson& req)
  {
    return isValid(Cmds::MaxRsp, req.at(Cmds::MaxReq), { {Param::required("name", JsonString)},
                                                         {Param::required("n",    JsonUInt)}});
  }
}
}

#endif

