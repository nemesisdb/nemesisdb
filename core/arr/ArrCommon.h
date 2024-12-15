#ifndef NDB_CORE_ARRCOMMON_H
#define NDB_CORE_ARRCOMMON_H

#include <core/NemesisCommon.h>


namespace nemesis {  namespace arr {


  enum class ArrQueryType : std::uint8_t
  { 
    Create,
    Delete,
    DeleteAll,
    Set,
    SetRng,
    Get,
    GetRng,
    Len,
    Used,    
    Exist,
    Clear,
    Swap,
    Intersect,
    Min,
    Max
  };


  template <class ArrayCmds>
  std::tuple<bool, njson> isArrayCmdValid ( const std::string_view queryRspName, 
                                            const njson& req,
                                            const ValidateParams& params,
                                            std::function<std::tuple<RequestStatus, const std::string_view>(const njson&)> onPostValidate = nullptr)
  {
    for (const auto& [member, param] : params)
    {
      if (param.variableType && !ArrayCmds::isTypeValid(req.at(member).type()))
        return {false, createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid)};
      else if (!param.variableType)
      {
        if (param.isRequired && !req.contains(member))
          return {false, createErrorResponse(queryRspName, RequestStatus::ParamMissing)};
        else if (req.contains(member) && req.at(member).type() != param.type)
          return {false, createErrorResponse(queryRspName, RequestStatus::ValueTypeInvalid)};
      }
    }

    if (onPostValidate)
    { 
      if (auto [stat, msg] = onPostValidate(req); stat == RequestStatus::Ok)
        return {true, njson{}};
      else
        return {false, createErrorResponse(queryRspName, stat, msg)};
    }
    else
      return {true, njson{}};
  }
}
}

#endif
