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
                                                                {"evt", json_array_arg_t{}}});


/// Holds all series, keyed by the series name.
/// This class requires that the JSON is validated before any function calls.
class Series
{ 

public:
  Series () = default;

  
  QueryResult create (const SeriesName& name, const std::string& cmdRspName)
  {
    if (hasSeries(name))
    {
      PLOGD << "Series name already exists";
      return TsRequestStatus::SeriesExists;
    }
    else
    {
      QueryResult qr{TsRequestStatus::Ok};

      m_series[name] = std::make_unique<OrderedSeries>(OrderedSeries::create(name));
      
      qr.rsp[cmdRspName]["st"] = toUnderlying(TsRequestStatus::Ok);
      qr.rsp[cmdRspName]["name"] = name;

      return qr;
    }
  }


  QueryResult add (njson query, const std::string& cmdRspName)
  {
    QueryResult qr{TsRequestStatus::Ok};

    const auto& name = query["ts"].as<SeriesName>();

    if (!hasSeries(name))
      qr.rsp[cmdRspName] = SeriesNotExistRsp;
    else
    {
      try
      {
        m_series.at(name)->add(query.at("t"), std::move(query.at("evt"))); 
        
        qr.rsp[cmdRspName]["st"] = toUnderlying(TsRequestStatus::Ok);
        qr.rsp[cmdRspName]["ts"] = name;
      }
      catch(const std::exception& ex)
      {
        PLOGE << ex.what();
        qr.status = TsRequestStatus::UnknownError;
      }
    }
    
    return qr;
  }


  QueryResult get (const njson& query, const std::string& cmdRspName) const
  {
    const auto& seriesName = query["ts"].as<SeriesName>();

    QueryResult result; 
    result.rsp[cmdRspName]["ts"] = seriesName;

    const auto& [stat, rsp] = getSingle(seriesName, makeGetParams(query), cmdRspName);
    
    result.rsp[cmdRspName].merge_or_update(std::move(rsp));
    result.status = stat;

    return result;
  }


  QueryResult getMultipleRanges (const njson& query, const std::string& cmdRspName) const
  {
    QueryResult result {TsRequestStatus::Ok}; // doesn't make sense for a query with multiple series

    // each key is a series name
    for (const auto& series : query.object_range())
    {
      const auto& name = series.key();

      const auto& [stat, rsp] = getSingle(name, makeGetParams(series.value()), cmdRspName);
      result.rsp[cmdRspName][name] = std::move(rsp);
    }

    return result;
  }


  bool hasSeries (const SeriesName& name) const
  {
    return m_series.contains(name);
  }


  QueryResult createIndex (const SeriesName& seriesName, const std::string& indexName, const std::string& cmdRspName)
  {
    QueryResult qr;

    if (!hasSeries(seriesName))
    {
      qr.status = TsRequestStatus::SeriesNotExist;
      qr.rsp = SeriesNotExistRsp;
    }
    else
    {
      const auto created = m_series.at(seriesName)->createIndex(indexName);

      PLOGD_IF(!created) << "Index '" << indexName << "' already exists in series " << seriesName; 

      qr.status = created ? TsRequestStatus::Ok : TsRequestStatus::IndexExists;  
      qr.rsp[cmdRspName][seriesName]["st"] = toUnderlying(qr.status);
    }
    return qr;
  }


private:


  GetParams makeGetParams (const njson& conditions) const
  {
    GetParams params{conditions.get_value_or<WhereType>("where", jsoncons::json_object_arg_t{})};

    if (const auto rngSize = conditions["rng"].size() ; rngSize)
    {
      // rng[0] is always start
      params.setStart(conditions["rng"][0].as<SeriesTime>());

      if (rngSize == 2U)
        params.setEnd(conditions["rng"][1].as<SeriesTime>());
    }
    
    return params;
  }


  std::tuple<TsRequestStatus,njson> getSingle(const SeriesName& name, const GetParams& params, const std::string& cmdRspName) const
  {
    if (!hasSeries(name))
      return {TsRequestStatus::SeriesNotExist, SeriesNotExistRsp};
    else
    {
      njson seriesRsp(json_object_arg, {{"st", toUnderlying(TsRequestStatus::Ok)},
                                        {"t", json_array_arg_t{}},
                                        {"evt", json_array_arg_t{}}});

      auto [stat, result] = m_series.at(name)->get(params);

      if (stat == TsRequestStatus::Ok)
        seriesRsp.merge_or_update(std::move(result));
      else
        seriesRsp["st"] = toUnderlying(stat);

      return {stat, seriesRsp};
    }
  }


private:
  std::map<const SeriesName, std::unique_ptr<BaseSeries>> m_series;
};


} // ns ts
} // ns core
} // ns nemesis

#endif
