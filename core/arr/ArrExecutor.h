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
  using ArrayValueT = Array::ValueT;


  static Response set (Array& array, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::SetRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    
    auto& rspBody = response.rsp.at(RspName);

    rspBody["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      if constexpr (Cmds::IsSorted)
      {
        if (array.isFull())
          rspBody["st"] = toUnderlying(RequestStatus::Bounds);
        else
          array.set(reqBody.at("item").as<ArrayValueT>());
      }
      else
      {
        const auto pos = reqBody.get_value_or<std::size_t>("pos", 0);
      
        if (!array.isInBounds(pos))
          rspBody["st"] = toUnderlying(RequestStatus::Bounds);
        else
          array.set(pos, reqBody.at("item").as<ArrayValueT>());
      }
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
    static const constexpr auto RspName = Cmds::SetRngRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto& items = reqBody.at("items").as<std::vector<ArrayValueT>>();

      if constexpr (Cmds::IsSorted)
      {
        if (!array.hasCapacity(items.size()))
          response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
        else
          array.setRange(items);
      }
      else
      {
        const auto pos = reqBody.at("pos").template as<std::size_t>();

        if (!array.isSetInBounds(pos, items.size()))
          response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
        else
          array.setRange(pos, items);
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response get (const Array& array, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::GetRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    const auto pos = reqBody.at("pos").as<std::size_t>();

    if (!array.isInBounds(pos))
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
    else
      response.rsp[RspName]["item"] = array.get(pos);

    return response;
  }


  static Response getRange (const Array& array, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::GetRngRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
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
      
      if (!array.isInBounds(start))  [[unlikely]]
        status = RequestStatus::Bounds;
      else
        items = array.getRange(start, stop);
    }
    
    response.rsp[RspName].try_emplace("items", std::move(items));
    response.rsp[RspName]["st"] = toUnderlying(status);

    return response;
  }


  static Response length (const Array& array, const njson& reqBody)
  {
    static const njson Prepared = njson{jsoncons::json_object_arg, {{Cmds::LenRsp.data(), njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[Cmds::LenRsp]["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp[Cmds::LenRsp]["len"] = array.size();
    return response;
  }


  static Response swap (Array& array, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::SwapRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto posA = reqBody.at("posA").as<std::size_t>();
      const auto posB = reqBody.at("posB").as<std::size_t>();
      
      if (!(array.isInBounds(posA) && array.isInBounds(posB)))
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
      else
        array.swap(posA, posB);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response clear (Array& array, const njson& reqBody)
  { 
    static const constexpr auto RspName = Cmds::ClearRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto& rng = reqBody.at("rng").array_range();
      const auto start = rng.cbegin()->as<std::size_t>();
      const auto stop = rng.crbegin()->as<std::size_t>();

      if (!array.isInBounds(start))
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
      else
        array.clear(start, stop);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response intersect (const Array& a, const Array& b) requires (Cmds::IsSorted)
  {
    static const constexpr auto RspName = Cmds::IntersectRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};

    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      std::vector<typename Cmds::ItemT> result;
      std::set_intersection(a.cbegin(), a.cend(),
                            b.cbegin(), b.cend(),
                            std::back_inserter(result));

      
      njson items (jsoncons::json_array_arg,  std::make_move_iterator(result.begin()),
                                              std::make_move_iterator(result.end()));
      
      response.rsp[RspName]["items"] = std::move(items);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }
};

}
}

#endif
