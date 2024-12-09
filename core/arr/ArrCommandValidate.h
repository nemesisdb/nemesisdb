#ifndef NDB_CORE_ARRCMDVALIDATE_H
#define NDB_CORE_ARRCMDVALIDATE_H

#include <tuple>
#include <string_view>
#include <core/NemesisCommon.h>
#include <core/arr/ArrCommands.h>


namespace nemesis { namespace arr {

  using namespace std::literals;

  using namespace nemesis::arr::cmds;
  using Validity = std::tuple<bool, njson>;


  Validity makeValid ()
  {
    const static Validity result {true, njson{}};
    return result;
  }
  

  Validity makeInvalid (njson err)
  {
    return Validity{false, std::move(err)};
  }


  template<typename Cmds>
  Validity validateCreate (const njson& request)
  {
    static const constexpr auto req = Cmds::CreateReq;
    static const constexpr auto rsp = Cmds::CreateRsp;
    
    auto [valid, err] = isValid(rsp, request.at(req), { {Param::required("name", JsonString)},
                                                        {Param::required("len", JsonUInt)}});

    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  Validity validateDelete (const njson& req)
  {
    if (auto [valid, err] = isValid(DeleteRsp, req.at(DeleteReq), {{Param::required("name", JsonString)}}); !valid)
      return makeInvalid(std::move(err));
    else
      return makeValid();
  }


  template<typename Cmds>
  Validity validateSet (const njson& req, const JsonType itemType)
  {
    auto [valid, err] = isValid(Cmds::SetRsp, req.at(Cmds::SetReq), { {Param::required("name", JsonString)},
                                                                      {Param::required("pos",  JsonUInt)},
                                                                      {Param::required("item", itemType)}});
    if (!valid)
      return makeInvalid(std::move(err));
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

    auto [valid, err] = isValid(SetRngRsp, req.at(SetRngReq), { {Param::required("name", JsonString)},
                                                                {Param::required("pos",  JsonUInt)},
                                                                {Param::required("items", JsonArray)}}, validate);
    if (!valid)
      return makeInvalid(std::move(err));
    else
      return makeValid();
  }


  Validity validateGet (const njson& req)
  {
    auto [valid, err] = isValid(GetRsp, req.at(GetReq), { {Param::required("name", JsonString)},
                                                          {Param::required("pos",  JsonUInt)}});
    if (!valid)
      return makeInvalid(std::move(err));
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
          const auto second = std::next(first);
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


    auto [valid, err] = isValid(GetRngRsp, req.at(GetRngReq), { {Param::required("name", JsonString)},
                                                                {Param::required("rng",  JsonArray)}}, validate);
    if (!valid)
      return makeInvalid(std::move(err));
    else
      return makeValid();
  }


  Validity validateLength (const njson& req)
  {
    auto [valid, err] = isValid(LenRsp, req.at(LenReq), { {Param::required("name", JsonString)}});
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  Validity validateSwap (const njson& req)
  {
    auto [valid, err] = isValid(SwapRsp, req.at(SwapReq), { {Param::required("name", JsonString)},
                                                            {Param::required("posA", JsonUInt)},
                                                            {Param::required("posB", JsonUInt)}});
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  Validity validateExist (const njson& req)
  {
    auto [valid, err] = isValid(ExistRsp, req.at(ExistReq), { {Param::required("name", JsonString)}});
    return valid ? makeValid() : makeInvalid(std::move(err));
  }


  Validity validateClear (const njson& req)
  {
    auto checkRange = [](const njson& body) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (body.at("rng").size() != 2)
        return {RequestStatus::CommandSyntax, "rng"};
      else
      {
        const auto rngArray = body.at("rng").array_range();
        const auto first = rngArray.begin();
        const auto second = std::next(first);

        if (!(first->is_uint64() && second->is_uint64()))
          return {RequestStatus::ValueTypeInvalid, "rng"};
      }

      return {RequestStatus::Ok, ""};
    };

    auto [valid, err] = isValid(ClearRsp, req.at(ClearReq), { {Param::required("name", JsonString)},
                                                              {Param::required("rng", JsonArray)}}, checkRange);
    return valid ? makeValid() : makeInvalid(std::move(err));
  }
}
}

#endif

