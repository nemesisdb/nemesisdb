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
using WhereType = std::string;
using WhereViewType = std::string_view;


enum class TsRequestStatus
{
  None = 0,
  Ok = 1,
  UnknownError,
  CommandNotExist,
  SeriesNotExist = 10,
  SeriesExists
};


enum class TsCommand
{
  TsCreate,
  TsAdd,
  TsGet,
  TsGetMultipleRanges,
  Max
};


const std::map<const std::string_view, const TsCommand> QueryNameToType = 
{  
  // session
  {"TS_CREATE",         TsCommand::TsCreate},
  {"TS_ADD",            TsCommand::TsAdd},
  {"TS_GET",            TsCommand::TsGet},
  {"TS_GET_MULTI",      TsCommand::TsGetMultipleRanges}
};


struct QueryResult
{
  QueryResult(const TsRequestStatus st = TsRequestStatus::None) : status(st)
  {
    
  }


  TsRequestStatus status;
  njson rsp;
};


struct GetParams
{
  GetParams (const std::string where = "") : where(where)
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
    return !(isStartSet() && isEndSet());
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
  BasicSeries ()
  {

  }

public:
  virtual void add (const SeriesTime td, const SeriesValue value) = 0;
  virtual void add (const njson& times, njson&& values) = 0;
  
  virtual njson get (const GetParams& params) const  = 0;
};

} // ns ts
} // ns core
} // ns nemesis

#endif
