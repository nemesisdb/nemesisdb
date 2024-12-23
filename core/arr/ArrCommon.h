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
  RequestStatus isArrayCmdValid ( const std::string_view queryRspName, 
                                            const njson& req,
                                            const ValidateParams& params,
                                            std::function<RequestStatus(const njson&)> onPostValidate = nullptr)
  {

    auto validate = [&req, &params]() -> RequestStatus
    {
      for (const auto& [member, param] : params)
      {
        if (param.variableType && !ArrayCmds::isTypeValid(req.at(member).type()))
          return RequestStatus::ValueTypeInvalid;
        else if (!param.variableType)
        {
          if (param.isRequired && !req.contains(member))
            return RequestStatus::ParamMissing;
          else if (req.contains(member) && req.at(member).type() != param.type)
            return RequestStatus::ValueTypeInvalid;
        }
      }

      return RequestStatus::Ok;
    };

    #ifdef NDB_DEBUG
      const auto status = validate();
      PLOGD_IF(status != RequestStatus::Ok) << "Status: " << toUnderlying(status);
      return status;
    #else
      return validate();
    #endif

    // if (onPostValidate)
    // { 
    //   if (const auto status = onPostValidate(req); status == RequestStatus::Ok)
    //     return {true, njson{}};
    //   else
    //     return {false, createErrorResponse(queryRspName, status)};
    // }
    // else
    //   return {true, njson{}};
  }
}
}

#endif
