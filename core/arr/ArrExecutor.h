#ifndef NDB_CORE_ARREXECUTOR_H
#define NDB_CORE_ARREXECUTOR_H

#include <functional>
#include <string_view>
#include <core/NemesisCommon.h>
#include <core/arr/ArrCommands.h>
#include <core/arr/ArrArray.h>


namespace nemesis { namespace arr {


namespace kvcmds = nemesis::arr::cmds;


template<class Array, class Cmds>
class ArrayExecutor
{
public:
  static Response set (Array& array, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::SetRsp.data();
    
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    
    auto& rspBody = response.rsp.at(RspName);

    rspBody["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto pos = reqBody.at("pos").as<std::size_t>();
      
      if (!array.isInBounds(pos))
        rspBody["st"] = toUnderlying(RequestStatus::Bounds);
      else
        array.set(pos, reqBody.at("item"));
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      rspBody["st"] = toUnderlying(RequestStatus::Unknown);
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

      if (!array.isSetInBounds(pos, items.size()))
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

    if (!array.isInBounds(pos))
      response.rsp[GetRsp]["st"] = toUnderlying(RequestStatus::Bounds);
    else
      response.rsp[GetRsp]["item"] = array.get(pos);

    return response;
  }


  static Response getRange (Array& array, const njson& reqBody)
  {
    static const njson Prepared = njson{jsoncons::json_object_arg, {{GetRngRsp, njson::object()}}};
    
    RequestStatus status = RequestStatus::Ok;
    Response response{.rsp = Prepared};

    const auto rngArray = reqBody.at("rng").array_range();
    const auto start = rngArray.begin()->as<std::size_t>();

    njson items;

    if (reqBody.at("rng").size() == 1)
    {
      if (!array.isInBounds(start))  [[unlikely]]
        status = RequestStatus::Bounds;
      else
        items = array.getRange(start);
    }
    else
    {
      const auto stop = std::next(rngArray.begin())->as<std::size_t>();
      
      if (!array.isGetInBounds(start, stop))  [[unlikely]]
        status = RequestStatus::Bounds;
      else
        items = array.getRange(start, stop);
    }
    
    response.rsp[GetRngRsp].try_emplace("items", std::move(items));
    response.rsp[GetRngRsp]["st"] = toUnderlying(status);

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
      
      if (!(array.isInBounds(posA) && array.isInBounds(posB)))
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


  static Response clear (Array& array, const njson& reqBody)
  {
    static const njson Prepared = njson{jsoncons::json_object_arg, {{ClearRsp, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[ClearRsp]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto& rng = reqBody.at("rng").array_range();
      const auto start = rng.cbegin()->as<std::size_t>();
      const auto stop = rng.crbegin()->as<std::size_t>();

      if (!array.isInBounds(start))
        response.rsp[ClearRsp]["st"] = toUnderlying(RequestStatus::Bounds);
      else
        array.clear(start, stop);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[ClearRsp]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }

};

}
}

#endif
