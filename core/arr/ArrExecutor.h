#ifndef NDB_CORE_ARREXECUTOR_H
#define NDB_CORE_ARREXECUTOR_H

#include <functional>
#include <string_view>
#include <core/NemesisCommon.h>
#include <core/arr/ArrCommands.h>
#include <core/arr/ArrArray.h>


namespace nemesis { namespace arr {


namespace kvcmds = nemesis::arr::cmds;


class ArrayExecutor
{
public:
  static Response set (Array& array, const njson& reqBody)
  {
    static const njson Prepared = njson{jsoncons::json_object_arg, {{SetRsp, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[SetRsp]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto pos = reqBody.at("pos").as<std::size_t>();
      
      if (!array.isInbounds(pos))
        response.rsp[SetRsp]["st"] = toUnderlying(RequestStatus::Bounds);
      else
      {
        array.set(pos, reqBody.at("item"));
      }        
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[SetRsp]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response setRange (Array& array, const njson& reqBody)
  {
    static const njson Prepared = njson{jsoncons::json_object_arg, {{SetRngRsp, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[SetRngRsp]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto pos = reqBody.at("pos").as<std::size_t>();
      const auto& items = reqBody.at("items");

      if (!array.isInbounds(pos+items.size()-1))
        response.rsp[SetRngRsp]["st"] = toUnderlying(RequestStatus::Bounds);
      else
        array.setRange(pos, items);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[SetRngRsp]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response get (Array& array, const njson& reqBody)
  {
    static const njson Prepared = njson{jsoncons::json_object_arg, {{GetRsp, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[GetRsp]["st"] = toUnderlying(RequestStatus::Ok);

    const auto pos = reqBody.at("pos").as<std::size_t>();

    if (!array.isInbounds(pos))
      response.rsp[GetRsp]["st"] = toUnderlying(RequestStatus::Bounds);
    else
      response.rsp[GetRsp]["item"] = array.get(pos);

    return response;
  }


  static Response getRange (Array& array, const njson& reqBody)
  {
    static const njson Prepared = njson{jsoncons::json_object_arg, {{GetRngRsp, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[GetRngRsp]["st"] = toUnderlying(RequestStatus::Ok);

    const auto rngArray = reqBody.at("rng").array_range();
    const auto start = rngArray.begin()->as<std::size_t>();
    const auto stop = std::next(rngArray.begin(), 1)->as<std::size_t>();

    PLOGD << "getRange(): " << start << " to " << stop;

    if (!(array.isInbounds(start) && array.isInbounds(stop)))
      response.rsp[GetRngRsp]["st"] = toUnderlying(RequestStatus::Bounds);
    else
      response.rsp[GetRngRsp].try_emplace("items", array.getRange(start, stop));

      //response.rsp[GetRngRsp]["items"] = array.getRange(start, stop);

    return response;
  }


  static Response length (Array& array, const njson& reqBody)
  {
    static const njson Prepared = njson{jsoncons::json_object_arg, {{LenRsp, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[LenRsp]["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp[LenRsp]["len"] = array.size();
    return response;
  }


  static Response swap (Array& array, const njson& reqBody)
  {
    static const njson Prepared = njson{jsoncons::json_object_arg, {{SwapRsp, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[SwapRsp]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto posA = reqBody.at("posA").as<std::size_t>();
      const auto posB = reqBody.at("posB").as<std::size_t>();
      
      if (!(array.isInbounds(posA) && array.isInbounds(posB)))
        response.rsp[SwapRsp]["st"] = toUnderlying(RequestStatus::Bounds);
      else
        array.swap(posA, posB);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[SwapRsp]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }
};

}
}

#endif
