#ifndef NDB_CORE_LSTEXECUTOR_H
#define NDB_CORE_LSTEXECUTOR_H

#include <functional>
#include <string_view>
#include <core/NemesisCommon.h>
#include <core/lst/LstCommands.h>
#include <core/lst/LstList.h>


namespace nemesis { namespace lst {


template<class List, class Cmds>
class ListExecutor
{
public:
  using ListValueT = List::ValueT;


  static Response add (List& list, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::AddRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto& items = reqBody.at("items").as<std::vector<ListValueT>>();
      
      if (reqBody.contains("pos"))
      {
        const auto pos = reqBody.at("pos").as<std::size_t>();

        const auto [insertedPos, size] = list.add(items, pos);
        response.rsp[RspName]["pos"] = insertedPos;
        response.rsp[RspName]["size"] = size;
      }
      else
      {
        const auto [insertedPos, size] = list.addTail(items);
        response.rsp[RspName]["pos"] = insertedPos;
        response.rsp[RspName]["size"] = size;
      }
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response get (const List& list, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::GetRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto pos = reqBody.at("pos").as<std::size_t>();

      if (!list.isInbounds(pos))  [[unlikely]]
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
      else
        response.rsp[RspName]["item"] = list.get(pos);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response getRange (const List& list, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::GetRngRsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto& rng = reqBody.at("rng").array_range().cbegin();
      const auto start = rng->as<std::size_t>();

      if (!list.isInbounds(start))
        response.rsp[RspName]["items"] = njson::make_array();
      else if (const auto rngSize = reqBody.at("rng").size(); rngSize == 1)
      {
        response.rsp[RspName]["items"] = list.getRange(start);
      }
      else
      {
        const auto stop = std::next(rng)->as<std::size_t>();
        response.rsp[RspName]["items"] = list.getRange(start, stop);
      }
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
