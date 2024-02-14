#ifndef NDB_CORE_TSCOMMON_H
#define NDB_CORE_TSCOMMON_H

#include <map>
#include <array>
#include <uwebsockets/App.h>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core { namespace ts {


using SeriesName = std::string;
using SeriesClock = chrono::steady_clock;
using SeriesDuration = chrono::microseconds;
using SeriesTimePoint = chrono::time_point<SeriesClock, SeriesDuration>;
using SeriesValue = njson;


struct GetParams
{
  SeriesDuration start;
  SeriesDuration end;
};

} // ns ts
} // ns core
} // ns nemesis

#endif
