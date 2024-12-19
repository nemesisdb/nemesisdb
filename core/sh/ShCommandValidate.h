#ifndef NDB_CORE_SHCMDVALIDATE_H
#define NDB_CORE_SHCMDVALIDATE_H

#include <tuple>
#include <core/NemesisCommon.h>
#include <core/sh/ShCommands.h>


namespace nemesis { namespace sh {

  using namespace nemesis::sh::cmds;


  RequestStatus validateNew (const njson& req)
  {
    const auto topLevelStatus = isValid(NewRsp, req.at(NewReq), {{Param::optional("expiry", JsonObject)}});

    if (topLevelStatus != RequestStatus::Ok)
      return topLevelStatus;
    else
    {
      if (req.at(NewReq).contains("expiry"))
      {
        auto validate = [](const njson& cmd) -> RequestStatus
        {
          if (cmd.at("duration") < 0)
            return RequestStatus::ValueSize;
          else
            return RequestStatus::Ok;
        };

        const auto expiryStatus = isValid(NewRsp, req.at(NewReq).at("expiry"), {{Param::required("duration", JsonUInt)},
                                                                                {Param::optional("deleteSession", JsonBool)},
                                                                                {Param::optional("extendOnSetAdd", JsonBool)},
                                                                                {Param::optional("extendOnGet", JsonBool)}
                                                                               }, validate);

        return expiryStatus;
      }
      else
        return RequestStatus::Ok; // no expiry, or has expiry and it's valid
    }
  }


  RequestStatus validateExists (const njson& req)
  {
    return isValid(ExistsRsp, req.at(ExistsReq), {{Param::required("tkns", JsonArray)}});
  }


  RequestStatus validateSave (const njson& req)
  {
    auto validate = [](const njson& cmd) -> RequestStatus
    {
      if (cmd.contains("tkns"))
      {
        if (cmd.at("tkns").empty())
          return RequestStatus::ValueSize;
        else
        {
          // check each array item is correct type, save command too important
          // to skip items that are incorrect type
          for (const auto& item : cmd.at("tkns").array_range())
          {
            if (!item.is<SessionToken>())
              return RequestStatus::ValueTypeInvalid;
          }
        }
      }

      return RequestStatus::Ok;
    };


    return isValid(SaveRsp, req.at(SaveReq), {{Param::required("name", JsonString)},
                                              {Param::optional("tkns", JsonArray)}}, validate);

  }


  RequestStatus validateLoad (const njson& req)
  {
    return isValid(LoadRsp, req.at(LoadReq), {{Param::required("name", JsonString)}});
  }
}
}

#endif

