#ifndef NDB_CORE_KVCMDVALIDATE_H
#define NDB_CORE_KVCMDVALIDATE_H

#include <tuple>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommands.h>


namespace nemesis { namespace kv {

  using namespace nemesis::kv;

  static RequestStatus validateSet (const std::string_view cmdReq, const std::string_view cmdRsp, const njson& req)
  {    
    return isValid(cmdRsp, req.at(cmdReq), {{Param::required("keys", JsonObject)}});
  }


  static RequestStatus validateGet(const std::string_view cmdReq, const std::string_view cmdRsp, const njson& req)
  {
    return isValid(cmdRsp, req.at(cmdReq), {{Param::required("keys", JsonArray)}});
  }


  static RequestStatus validateAdd(const std::string_view cmdReq, const std::string_view cmdRsp, const njson& req)
  {
    return isValid(cmdRsp, req.at(cmdReq), {{Param::required("keys", JsonObject)}});
  }


  static RequestStatus validateRemove(const std::string_view cmdReq, const std::string_view cmdRsp, const njson& req)
  {
    auto validate = [](const njson& cmd) -> RequestStatus
    {
      return cmd.at("keys").empty() ? RequestStatus::ValueSize : RequestStatus::Ok;
    };

    return isValid(cmdRsp, req.at(cmdReq), {{Param::required("keys", JsonArray)}}, validate);
  }


  static RequestStatus validateContains(const std::string_view cmdReq, const std::string_view cmdRsp, const njson& req)
  {
    return isValid(cmdRsp, req.at(cmdReq), {{Param::required("keys", JsonArray)}});
  }


  static RequestStatus validateFind(const std::string_view cmdReq, const std::string_view cmdRsp, const njson& req)
  {
    auto validate = [](const njson& cmd) -> RequestStatus
    {
      if (cmd.at("rsp") != "paths" && cmd.at("rsp") != "kv" && cmd.at("rsp") != "keys")
        return RequestStatus::CommandSyntax;
      else if (const auto& path = cmd.at("path").as_string() ; path.empty())
        return RequestStatus::ValueSize;

      return RequestStatus::Ok;
    };

    return isValid(cmdRsp, req.at(cmdReq), {  {Param::required("path", JsonString)},
                                              {Param::required("rsp", JsonString)},
                                              {Param::optional("keys", JsonArray)}}, validate);
  }


  static RequestStatus validateUpdate(const std::string_view cmdReq, const std::string_view cmdRsp, const njson& req)
  {
    auto validate = [](const njson& cmd) -> RequestStatus
    {
      // 'value' can be any valid JSON type, so just check it's present here
      if (!cmd.contains("value"))
        return RequestStatus::ParamMissing;
      
      return RequestStatus::Ok;
    };

    return isValid(cmdRsp, req.at(cmdReq), { {Param::required("key", JsonString)},
                                             {Param::required("path", JsonString)}}, validate);
  }


  static RequestStatus validateClearSet(const std::string_view cmdReq, const std::string_view cmdRsp, const njson& req)
  {
    return isValid(cmdRsp, req.at(cmdReq), {{Param::required("keys", JsonObject)}});
  }


  static RequestStatus validateArrayAppend(const std::string_view cmdReq, const std::string_view cmdRsp, const njson& req)
  {
    return isValid(cmdRsp, req.at(cmdReq), {{Param::required("key", JsonString)},
                                            {Param::required("items", JsonArray)}, 
                                            {Param::optional("name", JsonString)}});
  }


  static RequestStatus validateSave(const njson& req)
  {
    return isValid(kv::cmds::SaveRsp, req.at(kv::cmds::SaveReq), {{Param::required("name", JsonString)}});
  }


  static RequestStatus validateLoad(const njson& req)
  {
    return isValid(kv::cmds::LoadRsp, req.at(kv::cmds::LoadReq), {{Param::required("name", JsonString)}});
  } 
}
}

#endif

