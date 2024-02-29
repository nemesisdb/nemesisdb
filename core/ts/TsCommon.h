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





enum class TsRequestStatus
{
  None = 0,
  Ok = 1,
  UnknownError,
  CommandNotExist,
  ParamMissing,
  ParamType,
  RngSize,
  RngValues,
  SeriesNotExist = 20,
  SeriesExists,
  SeriesType,
  IndexExists = 40
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
  using TimeIndexTuple = std::tuple<SeriesTime, std::size_t>;
  using IndexedTimes = std::vector<TimeIndexTuple>;  

  struct IndexComparer
  {
    bool operator()(const TimeIndexTuple& a, const TimeIndexTuple& b)
    {
      return std::get<1>(a) < std::get<1>(b);
    }
  };

  struct IndexTimeComparer
  {
    IndexTimeComparer (const SeriesTime& min, const SeriesTime& max) : min(min), max(max)
    {

    }

    bool operator()(const TimeIndexTuple& a, const TimeIndexTuple& b)
    {
      return  std::get<1>(a) < std::get<1>(b) &&
              (std::get<0>(a) >= min && std::get<0>(a) < max) &&
              (std::get<0>(b) >= min && std::get<0>(b) < max);
    }

    SeriesTime min, max;
  };

  IndexNode () = default;

  IndexNode (const SeriesTime time, const std::size_t index)
  {
    times.emplace_back(time, index);
  }


  void add (const SeriesTime time, const std::size_t index)
  {
    times.emplace_back(time, index);
    std::sort(std::begin(times), std::end(times), IndexComparer{});
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
  GetParams (const WhereType where = njson{jsoncons::json_object_arg}) : where(where)
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


class BaseSeries
{
protected:
  BaseSeries () = default;

public:
  virtual ~BaseSeries() = default;

  virtual void add (const njson& times, njson&& values) = 0;
  
  virtual njson get (const GetParams& params) const  = 0;

  virtual bool createIndex (const std::string& key) = 0;
};

} // ns ts
} // ns core
} // ns nemesis

#endif
