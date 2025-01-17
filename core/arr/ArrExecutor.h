#ifndef NDB_CORE_ARREXECUTOR_H
#define NDB_CORE_ARREXECUTOR_H

#include <functional>
#include <string_view>
#include <core/NemesisCommon.h>
#include <core/arr/ArrCommands.h>
#include <core/arr/ArrArray.h>


namespace nemesis { namespace arr {


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
        if (reqBody.contains("pos"))
        {
          const std::size_t pos = reqBody.at("pos").as<std::size_t>();
      
          if (!array.isInBounds(pos))
            rspBody["st"] = toUnderlying(RequestStatus::Bounds);
          else
            array.set(pos, reqBody.at("item").as<ArrayValueT>());
        }
        else
        {
          if (array.isFull())
            rspBody["st"] = toUnderlying(RequestStatus::Bounds);
          else
            array.set(reqBody.at("item").as<ArrayValueT>());
        }
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      rspBody["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response setRange (Array& array, njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::SetRngRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      // TODO is there an alternative to this
      auto items = reqBody.at("items").as<std::vector<ArrayValueT>>();

      if constexpr (Cmds::IsSorted)
      {
        if (!array.hasCapacity(items.size()))
          response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
        else
          array.setRange(items);
      }
      else
      {
        if (reqBody.contains("pos"))
        {
          const std::size_t pos = reqBody.at("pos").as<std::size_t>();

          if (!array.isSetInBounds(pos, items.size()))
            response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
          else
            array.setRange(pos, items);
        }
        else
        {
          if (array.isFull())
            response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
          else
            array.setRange(items);
        }
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
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    const auto [start, stop, hasStop, hasRng] = rangeFromRequest(reqBody, "rng");

    if (!array.isInBounds(start))  [[unlikely]]
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
    else
      response.rsp[RspName]["items"] = array.getRange(start, hasStop ? stop : array.size());

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


  static Response used (const Array& array, const njson& reqBody)
  {
    static const njson Prepared = njson{jsoncons::json_object_arg, {{Cmds::UsedRsp.data(), njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[Cmds::UsedRsp]["st"] = toUnderlying(RequestStatus::Ok);
    response.rsp[Cmds::UsedRsp]["used"] = array.used();
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
      const auto [start, stop, hasStop, hasRng] = rangeFromRequest(reqBody, "rng");
      
      if (!array.isInBounds(start))
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
      else
        array.clear(start, hasStop ? stop : array.size());
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


  static Response min (Array& array, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::MinRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto n = reqBody.at("n").as<std::size_t>();
      
      if (array.isEmpty())
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
      else
        response.rsp[RspName]["items"] = array.min(n);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response max (Array& array, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::MaxRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto n = reqBody.at("n").as<std::size_t>();
      
      if (array.isEmpty())
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
      else
        response.rsp[RspName]["items"] = array.max(n);
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
