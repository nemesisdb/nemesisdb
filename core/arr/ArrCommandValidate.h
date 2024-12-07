#ifndef NDB_CORE_ARRCMDVALIDATE_H
#define NDB_CORE_ARRCMDVALIDATE_H

#include <tuple>
#include <core/NemesisCommon.h>
#include <core/arr/ArrCommands.h>


namespace nemesis { namespace arr {

  using namespace nemesis::arr::cmds;
  using Validity = std::tuple<bool, njson>;


  Validity makeValid ()
  {
    const static Validity result {true, njson{}};
    return result;
  }
  

  Validity makeInvalid (njson rsp)
  {
    return Validity{false, std::move(rsp)};
  }


  Validity validateCreate (const njson& req)
  {
    auto [valid, rsp] = isValid(CreateRsp, req.at(CreateReq), { {Param::required("name", JsonString)},
                                                                {Param::required("len", JsonUInt)}});

    if (!valid)
      return makeInvalid(std::move(rsp));
    else
      return makeValid();
  }


  Validity validateDelete (const njson& req)
  {
    if (auto [valid, rsp] = isValid(DeleteRsp, req.at(DeleteReq), {{Param::required("name", JsonString)}}); !valid)
      return makeInvalid(std::move(rsp));
    else
      return makeValid();
  }


  Validity validateSet (const njson& req)
  {
    auto [valid, rsp] = isValid(SetRsp, req.at(SetReq), { {Param::required("name", JsonString)},
                                                          {Param::required("pos",  JsonUInt)},
                                                          {Param::required("item", JsonObject)}});
    if (!valid)
      return makeInvalid(std::move(rsp));
    else
      return makeValid();
  }


  Validity validateSetRange (const njson& req)
  {
    auto validate = [](const njson& body) -> std::tuple<RequestStatus, const std::string_view>
    {
      for(const auto& item : body.at("items").array_range())
        if (!item.is_object())
          return {RequestStatus::ValueTypeInvalid, "items"};
      
      return {RequestStatus::Ok, ""};
    };

    auto [valid, rsp] = isValid(SetRngRsp, req.at(SetRngReq), { {Param::required("name", JsonString)},
                                                                {Param::required("pos",  JsonUInt)},
                                                                {Param::required("items", JsonArray)}}, validate);
    if (!valid)
      return makeInvalid(std::move(rsp));
    else
      return makeValid();
  }


  Validity validateGet (const njson& req)
  {
    auto [valid, rsp] = isValid(GetRsp, req.at(GetReq), { {Param::required("name", JsonString)},
                                                          {Param::required("pos",  JsonUInt)}});
    if (!valid)
      return makeInvalid(std::move(rsp));
    else
      return makeValid();
  }


  Validity validateGetRange (const njson& req)
  {
    auto validate = [](const njson& body) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (const auto nDims = body.at("rng").size(); !(nDims == 1 || nDims == 2)) [[unlikely]]
        return {RequestStatus::ValueSize, "rng"};
      else
      {
        const auto rngArray = body.at("rng").array_range();
        const auto first = rngArray.begin();
        
        if (!first->is_uint64())
          return {RequestStatus::ValueTypeInvalid, "rng"};
        else if (nDims == 2)
        {
          const auto second = std::next(rngArray.begin());
          if (!second->is_uint64())
            return {RequestStatus::ValueTypeInvalid, "rng"};
          else
          {
            const auto start = first->as<std::size_t>();
            const auto stop = second->as<std::size_t>();

            if (start > stop)
              return {RequestStatus::CommandSyntax, "start > stop"};
          }
        }
      }
      
      return {RequestStatus::Ok, ""};
    };


    auto [valid, rsp] = isValid(GetRngRsp, req.at(GetRngReq), { {Param::required("name", JsonString)},
                                                                {Param::required("rng",  JsonArray)}}, validate);
    if (!valid)
      return makeInvalid(std::move(rsp));
    else
      return makeValid();
  }


  Validity validateLength (const njson& req)
  {
    if (auto [valid, rsp] = isValid(LenRsp, req.at(LenReq), { {Param::required("name", JsonString)}}); !valid)
      return makeInvalid(std::move(rsp));
    else
      return makeValid();
  }


  Validity validateSwap (const njson& req)
  {
    auto [valid, rsp] = isValid(SwapRsp, req.at(SwapReq), { {Param::required("name", JsonString)},
                                                            {Param::required("posA", JsonUInt)},
                                                            {Param::required("posB", JsonUInt)}});
    if (!valid)
      return makeInvalid(std::move(rsp));
    else
      return makeValid();
  }
}
}

#endif

