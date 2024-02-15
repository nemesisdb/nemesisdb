#ifndef NDB_CORE_TSCOMMON_H
#define NDB_CORE_TSCOMMON_H

#include <map>
#include <array>
#include <uwebsockets/App.h>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core { namespace ts {


using SeriesName = std::string;
using SeriesClock = chrono::steady_clock;
//using SeriesDuration = chrono::milliseconds;
//using SeriesTimePoint = chrono::time_point<SeriesClock, SeriesDuration>;
using SeriesTime = chrono::milliseconds::rep;
using SeriesValue = njson;

enum class TsStatus
{
  None = 0,
  Ok = 1,
  SeriesNotExist = 10
};


struct QueryResult
{
  QueryResult(const TsStatus st = TsStatus::None) : status(st)
  {
    
  }


  TsStatus status;
  njson rsp;
};


struct GetParams
{
  SeriesTime start{0};
  SeriesTime end{0};
};


// TODO crap name, but Series is used instead of "SeriesManager"
class BasicSeries
{
protected:
  BasicSeries ()
  {

  }

public:
  virtual void add (const SeriesTime td, const SeriesValue value) = 0;
  virtual void add (const njson& times, njson&& values) = 0;
  
  virtual njson get (const GetParams& params) = 0;
};

} // ns ts
} // ns core
} // ns nemesis

#endif
