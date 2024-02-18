#ifndef NDB_CORE_TSSERIES_H
#define NDB_CORE_TSSERIES_H

#include <vector>
#include <memory>
#include <map>
#include <core/ts/TsCommon.h>
#include <core/ts/OrderedSeries.h>


namespace nemesis { namespace core { namespace ts {


using namespace jsoncons;
using namespace jsoncons::literals;


static inline const njson SeriesNotExistRsp (json_object_arg, { {"st", toUnderlying(TsRequestStatus::SeriesNotExist)},
                                                                {"t", json_array_arg_t{}},
                                                                {"v", json_array_arg_t{}}});


/// Holds all series, keyed by the series name.
/// This class requires that the JSON types are validated before any function calls.
class Series
{ 

public:
  Series () 
  {

  } 

  
  QueryResult create (const std::string& name)
  {
    if (containsSeries(name))
      return TsRequestStatus::SeriesExists;
    else
    {
      QueryResult qr;

      m_series[name] = std::make_unique<OrderedSeries>(OrderedSeries::create(name));
      
      qr.rsp["TS_CREATE_RSP"]["st"] = toUnderlying(TsRequestStatus::Ok);
      qr.rsp["TS_CREATE_RSP"]["name"] = name;

      return qr;
    }
  }


  QueryResult add (njson&& query)
  {
    QueryResult qr{TsRequestStatus::Ok};

    const auto& name = query["ts"].as<SeriesName>();

    try
    {
      m_series.at(name)->add(query.at("t"), std::move(query.at("v"))); 
      
      qr.rsp["TS_ADD_RSP"]["st"]  = toUnderlying(TsRequestStatus::Ok);
      qr.rsp["TS_ADD_RSP"]["ts"] = name;
    }
    catch(const std::exception& ex)
    {
      PLOGE << ex.what();
      qr.status = TsRequestStatus::UnknownError;
    }
    
    return qr;
  }


  QueryResult get (const njson& query) const
  {
    const auto& names = query["ts"].as<std::vector<SeriesName>>();
    const auto where = query.get_value_or<std::string>("where", "");
    const auto start = query["rng"][0].as<SeriesTime>();
    const auto end = query["rng"].size() == 2U ? query["rng"][1].as<SeriesTime>() : 0U;

    QueryResult result {TsRequestStatus::Ok}; // TODO doesn't make sense for a query with multiple parts (multiple series)

    for (const auto& name : names)
      result.rsp[name] = getSingle(name, GetParams{.start = start, .end = end, .where = where});

    return result;
  }


  QueryResult getMultipleRanges (const njson& query) const
  {
    QueryResult result {TsRequestStatus::Ok}; // doesn't make sense for a query with multiple series

    // each key is a series name
    for (const auto& member : query.object_range())
    {
      const auto& name = member.key();
      const auto start = member.value()["rng"][0].as<SeriesTime>();
      const auto end = member.value()["rng"].size() == 2U ? member.value()["rng"][1].as<SeriesTime>() : 0U;

      result.rsp[member.key()] = getSingle(name, GetParams{.start = start, .end = end});
    }

    return result;
  }


  bool containsSeries (const SeriesName& name) const
  {
    return m_series.contains(name);
  }


private:

  njson getSingle(const SeriesName& name, const GetParams& params) const
  {
    if (!containsSeries(name))
      return SeriesNotExistRsp;
    else
    {
      njson seriesRsp(json_object_arg, {{"st", toUnderlying(TsRequestStatus::Ok)},
                                        {"t", json_array_arg_t{}},
                                        {"v", json_array_arg_t{}}});

      seriesRsp.merge_or_update(m_series.at(name)->get(params));

      return seriesRsp;
    }
  }


private:
  std::map<const SeriesName, std::unique_ptr<BasicSeries>> m_series;
};


} // ns ts
} // ns core
} // ns nemesis

#endif
