

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
  // TODO use in ArrExecutor
  struct Rng
  {    
    std::size_t start;
    std::size_t stop{};
    bool hasStop{false};
    bool hasRng{false};
  };


public:
  using ListValueT = List::ValueT;


  static Response add (List& list, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::add.rsp.data();
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


  static Response setRange (List& list, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::setRng.rsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};

    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto pos = reqBody.at("pos").as<std::size_t>();

      if (!list.isInbounds(pos))  [[unlikely]]
        response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Bounds);
      else
        list.setRange(reqBody.at("items"), pos);
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
    static const constexpr auto RspName = Cmds::get.rsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      std::size_t pos{};

      if (reqBody.contains("pos"))
        pos = reqBody.at("pos").as<std::size_t>();
      else if (const auto size = list.size(); size) 
        pos = size-1; // tail

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
    static const constexpr auto RspName = Cmds::getRng.rsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};
    response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

    try
    {
      const auto [start, stop, hasStop, hasRange] = rangeFromRequest(reqBody, "rng");

      if (!list.isInbounds(start))
        response.rsp[RspName]["items"] = njson::make_array();
      else if (!hasStop)
        response.rsp[RspName]["items"] = list.getRange(start);
      else
        response.rsp[RspName]["items"] = list.getRange(start, stop);
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response remove (List& list, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::remove.rsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};

    try
    {
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);

      if (reqBody.contains("head") && reqBody.at("head").as_bool())
        list.removeHead();
      
      if (reqBody.contains("tail") && reqBody.at("tail").as_bool())
        list.removeTail();

      const auto [start, stop, hasStop, hasRange] = rangeFromRequest(reqBody, "rng");

      if (hasRange)
      {
        hasStop ? list.remove(start, stop) : list.remove(start);
      }

      response.rsp[RspName]["size"] = list.size();
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


  static Response length (const List& list, const njson& reqBody)
  {
    static const constexpr auto RspName = Cmds::len.rsp.data();
    static const njson Prepared = njson{jsoncons::json_object_arg, {{RspName, njson::object()}}};
    
    Response response{.rsp = Prepared};

    try
    {
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Ok);
      response.rsp[RspName]["len"] = list.size();
    }
    catch(const std::exception& e)
    {
      PLOGE << e.what();
      response.rsp[RspName]["st"] = toUnderlying(RequestStatus::Unknown);
    }

    return response;
  }


private:
  static Rng rangeFromRequest(const njson& body, const std::string_view name)
  {
    if (!body.contains(name))
      return {};
    else
    {
      Rng result{.hasRng = true};

      const auto& element = body.at(name);
      const auto& rng = element.array_range();
      
      result.start = rng.cbegin()->as<std::size_t>();
      
      if (element.size() == 2)
      {
        result.stop = rng.crbegin()->as<std::size_t>();
        result.hasStop = true;
      }

      return result;
    }      
  }

private:
  

};

}
}

#endif
