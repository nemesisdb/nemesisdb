#ifndef NDB_CORE_TSCOMMON_H
#define NDB_CORE_TSCOMMON_H

#include <map>
#include <array>
#include <functional>
#include <uwebsockets/App.h>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core { namespace ts {


using SeriesName = std::string;
using SeriesTime = std::int64_t;
using SeriesValue = njson;
using WhereType = njson;
//using WhereViewType = std::string_view;


enum class TsRequestStatus
{
  None = 0,
  Ok = 1,
  UnknownError,
  CommandNotExist,
  SeriesNotExist = 10,
  SeriesExists,
  IndexExists = 20
};


enum class TsCommand
{
  TsCreate,
  TsAdd,
  TsGet,
  TsGetMultipleRanges,
  TsCreateIndex,
  Max
};


const std::map<const std::string_view, const TsCommand> QueryNameToType = 
{  
  {"TS_CREATE",         TsCommand::TsCreate},
  {"TS_ADD",            TsCommand::TsAdd},
  {"TS_GET",            TsCommand::TsGet},
  {"TS_GET_MULTI",      TsCommand::TsGetMultipleRanges},
  {"TS_CREATE_INDEX",   TsCommand::TsCreateIndex}
};


struct QueryResult
{
  QueryResult(const TsRequestStatus st = TsRequestStatus::None) : status(st)
  {
    
  }


  TsRequestStatus status;
  njson rsp;
};


struct IndexNode
{
  using IndexedTimes = std::vector<std::tuple<SeriesTime, std::size_t>>;  

  IndexNode () = default;

  IndexNode (const SeriesTime time, const std::size_t index)
  {
    times.emplace_back(time, index);
  }


  void add (const SeriesTime time, const std::size_t index)
  {
    times.emplace_back(time, index);

    // keep times vector sorted by the index
    // const auto insertIt = std::lower_bound(times.cbegin(), times.cend(), index, [](const auto& timeToIndex, const std::size_t& index)
    // {
    //   return std::get<1>(timeToIndex) < index;
    // });

    // times.emplace(insertIt, time, index);
  }


  const IndexedTimes& getTimes () const
  {
    return times;
  }

  
  private:
    IndexedTimes times;
};


struct Index
{
  using IndexMap = std::map<njson, IndexNode> ;
  using IndexMapConstIt = std::map<njson, IndexNode>::const_iterator ;

  std::map<njson, IndexNode> index;
};





struct GetParams
{
  GetParams (const WhereType where = jsoncons::json_object_arg_t{}) : where(where)
  {
  }

  bool isStartSet () const
  {
    return startSet;
  }

  bool isEndSet () const
  {
    return endSet;
  }

  void setStart (const SeriesTime s)
  {
    start = s;
    startSet = true;
  }

  void setEnd (const SeriesTime e)
  {
    end = e;
    endSet = true;
  }


  bool isFullRange () const
  {
    return !isStartSet() && !isEndSet();
  }
  

  SeriesTime getStart() const
  {
    return start;
  }
  
  SeriesTime getEnd() const
  {
    return end;
  }

  WhereType getWhere() const
  {
    return where;
  }

  bool hasWhere() const
  {
    return !where.empty();
  }

private:
  bool startSet{false};
  bool endSet{false};
  SeriesTime start{0};
  SeriesTime end{0};
  WhereType where;
};


using TsWebSocket = uWS::WebSocket<false, true, WsSession>;

// TODO crap name, but Series is already used (instead of "SeriesManager")
class BasicSeries
{
protected:
  BasicSeries () = default;

public:
  virtual ~BasicSeries() = default;

  virtual void add (const njson& times, njson&& values) = 0;
  
  virtual njson get (const GetParams& params) const  = 0;

  virtual bool createIndex (const std::string& key) = 0;
};

} // ns ts
} // ns core
} // ns nemesis

#endif
