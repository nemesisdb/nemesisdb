#ifndef NDB_CORE_SHCMDVALIDATE_H
#define NDB_CORE_SHCMDVALIDATE_H

#include <tuple>
#include <core/NemesisCommon.h>
#include <core/sh/ShCommands.h>


namespace nemesis { namespace core { namespace sh {


  using Validity = std::tuple<bool, njson>;


  Validity makeValid ()
  {
    return Validity{true, njson{}};
  }

  Validity makeInvalid (njson rsp)
  {
    return Validity{false, std::move(rsp)};
  }


  Validity validateNew (njson& req)
  {
    if (auto [topLevelValid, rsp] = isValid(sh::NewRsp, req.at(sh::NewReq), {{Param::optional("expiry", JsonObject)}}); !topLevelValid)
      return Validity{false, std::move(rsp)};
    else
    {
      if (req.at(sh::NewReq).contains("expiry"))
      {
        auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
        {
          if (cmd.at("duration") < 0)
            return {RequestStatus::ValueSize, "expiry must be >= 0"};
          else
            return {RequestStatus::Ok, ""};
        };

        
        auto [expiryValid, rsp] = isValid(sh::NewRsp, req.at(sh::NewReq).at("expiry"), {{Param::required("duration", JsonUInt)},
                                                                                        {Param::required("deleteSession", JsonBool)}}, validate);

        if (!expiryValid)
          return makeInvalid(std::move(rsp));
      }

      // no expiry, or has expiry and it's valid
      return makeValid();
    }
  }


  Validity validateExists (njson& req)
  {
    if (auto [valid, rsp] = isValid(sh::ExistsRsp, req.at(sh::ExistsReq), {{Param::required("tkns", JsonArray)}}); !valid)
      return makeInvalid(std::move(rsp));
    else
      return makeValid();
  }


  Validity validateSave (njson& req)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.contains("tkns"))
      {
        if (cmd.at("tkns").empty())
          return {RequestStatus::ValueSize, "'tkns' empty"};
        else
        {
          // check each array item is correct type, save command too important
          // to skip items that are incorrect type
          for (const auto& item : cmd.at("tkns").array_range())
          {
            if (!item.is<SessionToken>())
              return {RequestStatus::ValueTypeInvalid, "'tkns' contains invalid token"};
          }
        }
      }

      return {RequestStatus::Ok, ""};
    };


    auto [valid, rsp] = isValid(sh::SaveRsp, req.at(sh::SaveReq), { {Param::required("name", JsonString)},
                                                                    {Param::optional("tkns", JsonArray)}}, validate);

    if (!valid)
      return makeInvalid(std::move(rsp));
    else
      return makeValid();
  }


  Validity validateLoad (njson& req)
  {
    if (auto [valid, rsp] = isValid(sh::LoadRsp, req.at(sh::LoadReq), {{Param::required("name", JsonString)}}); !valid)
      return makeInvalid(std::move(rsp));
    else
      return makeValid();
  }
}
}
}

#endif

