#ifndef NDB_CORE_KVCMDVALIDATE_H
#define NDB_CORE_KVCMDVALIDATE_H

#include <tuple>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommands.h>


namespace nemesis { namespace core { namespace kv {


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


  Validity validateSet (const njson& req)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };
    
    if (auto [valid, rsp] = isValid(SetRsp, req.at(kv::SetReq), {{Param::required("keys", JsonObject)}}, validate); !valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateSetQ(const njson& req)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };
    
    if (auto [valid, rsp] = isValid(SetQRsp, req.at(kv::SetQReq), {{Param::required("keys", JsonObject)}}, validate); !valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateGet(const njson& req)
  {
    if (auto [valid, rsp] = isValid(GetReq, req.at(kv::GetReq), {{Param::required("keys", JsonArray)}}); !valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateAdd(const njson& req)
  {
    if (auto [valid, rsp] = isValid(AddRsp, req.at(kv::AddReq), {{Param::required("keys", JsonObject)}}); !valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateAddQ(const njson& req)
  {
    if (auto [valid, rsp] = isValid(AddQRsp, req.at(kv::AddQReq), {{Param::required("keys", JsonObject)}}); !valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateRemove(const njson& req)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      return cmd.at("keys").empty() ? std::make_tuple(RequestStatus::ValueSize, "keys") : std::make_tuple(RequestStatus::Ok, "");
    };

    if (auto [valid, rsp] = isValid(RmvRsp, req.at(kv::RmvReq), {{Param::required("keys", JsonArray)}}, validate); !valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateContains(const njson& req)
  {
    if (auto [valid, rsp] = isValid(ContainsRsp, req.at(kv::ContainsReq), {{Param::required("keys", JsonArray)}}); !valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateFind(const njson& req)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      if (cmd.at("rsp") != "paths" && cmd.at("rsp") != "kv" && cmd.at("rsp") != "keys")
        return {RequestStatus::CommandSyntax, "'rsp' invalid value"};
      else if (const auto& path = cmd.at("path").as_string() ; path.empty())
        return {RequestStatus::ValueSize, "'path' is empty"};

      return {RequestStatus::Ok, ""};
    };

    auto [valid, rsp] = isValid(FindRsp, req.at(kv::FindReq), { {Param::required("path", JsonString)},
                                                                {Param::required("rsp", JsonString)},
                                                                {Param::optional("keys", JsonArray)}}, validate);
    if (!valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateUpdate(const njson& req)
  {
    auto validate = [](const njson& cmd) -> std::tuple<RequestStatus, const std::string_view>
    {
      // 'value' can be any valid JSON type, so just check it's present here
      if (!cmd.contains("value"))
        return {RequestStatus::ParamMissing, "Missing parameter"};
      
      return {RequestStatus::Ok, ""};
    };

    auto [valid, rsp] = isValid(UpdateRsp, req.at(kv::UpdateReq), { {Param::required("key", JsonString)},
                                                                    {Param::required("path", JsonString)}}, validate);

    if (!valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateClearSet(const njson& req)
  {
    if (auto [valid, rsp] = isValid(ClearSetRsp, req.at(kv::ClearSetReq), {{Param::required("keys", JsonObject)}}); !valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateArrayAppend(const njson& req)
  {
    auto [valid, rsp] = isValid(ArrAppendRsp, req.at(kv::ArrAppendReq), { {Param::required("key", JsonString)},
                                                                          {Param::required("items", JsonArray)}, 
                                                                          {Param::optional("name", JsonString)}});
    if (!valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateSave(const njson& req)
  {
    if (auto [valid, rsp] = isValid(kv::SaveRsp, req.at(kv::SaveReq), {{Param::required("name", JsonString)}}); !valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }


  Validity validateLoad(const njson& req)
  {
    if (auto [valid, rsp] = isValid(kv::LoadRsp, req.at(kv::LoadReq), {{Param::required("name", JsonString)}}); !valid)
      return makeInvalid(std::move(rsp));
    else  [[likely]]
      return makeValid();
  }

  
}
}
}

#endif

