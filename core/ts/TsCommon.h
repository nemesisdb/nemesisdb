#ifndef NDB_CORE_TSCOMMON_H
#define NDB_CORE_TSCOMMON_H

#include <map>
#include <array>
#include <functional>
#include <uwebsockets/App.h>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core { namespace ts {


using SeriesName = std::string;
using SeriesClock = chrono::steady_clock;
//using SeriesDuration = chrono::milliseconds;
//using SeriesTimePoint = chrono::time_point<SeriesClock, SeriesDuration>;
using SeriesTime = chrono::milliseconds::rep;
using SeriesValue = njson;


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
  SeriesTime start{0};
  SeriesTime end{0};
  std::string where;
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
