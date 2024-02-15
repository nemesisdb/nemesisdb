#ifndef NDB_CORE_TSSERIES_H
#define NDB_CORE_TSSERIES_H

#include <vector>
#include <memory>
#include <map>
#include <core/ts/TsCommon.h>


namespace nemesis { namespace core { namespace ts {


using namespace jsoncons;
using namespace jsoncons::literals;


///
class Series
{ 

public:
  Series () 
  {

  } 

  
  bool create (const std::string& name)
  {
    if (containsSeries(name))
      return false;
    else
    {
      m_series[name] = std::make_unique<OrderedSeries>(OrderedSeries::create(name));
      return true;
    }
  }


  QueryResult add (njson& query)
  {
    const auto& name = query["ts"].as<SeriesName>();

    m_series.at(name)->add(query.at("t"), std::move(query.at("v")));
    
    return TsStatus::Ok;
  }


  QueryResult get (const njson& query)
  {
    const auto& name = query["ts"].as_string();
    const auto start = query["rng"][0].as<SeriesTime>();
    const auto end = query["rng"].size() == 2 ? query["rng"][1].as<SeriesTime>() : 0U;

    QueryResult result;
    result.status = TsStatus::Ok;
    result.rsp = m_series.at(name)->get(GetParams{.start = start, .end = end});

    return result;
  }


  bool containsSeries (const SeriesName& name)
  {
    return m_series.contains(name);
  }


private:
  std::map<const SeriesName, std::unique_ptr<BasicSeries>> m_series;


};


} // ns ts
} // ns core
} // ns nemesis

#endif
