#ifndef NDB_CORE_TSORDEREDSERIES_H
#define NDB_CORE_TSORDEREDSERIES_H

#include <vector>
#include <map>
#include <core/ts/TsCommon.h>


namespace nemesis { namespace core { namespace ts {


using namespace jsoncons;
using namespace jsoncons::literals;


/// Represents a time series that submitted data is always gauranteed to be ordered by the sender. 
/// Uses parallel vectors to hold time and value data:
///   An event occurring at m_times[i] has metrics in m_values[i].
class OrderedSeries : public BasicSeries
{
  using TimeVector = std::vector<SeriesTime>;
  using ValueVector = std::vector<SeriesValue>;

  using TimeVectorConstIt = TimeVector::const_iterator;
  using ValueVectorConstIt = ValueVector::const_iterator;

  using Indexes = std::map<std::string, Index>;
  using IndexesIt = Indexes::iterator;
  using IndexesConstIt = Indexes::const_iterator;


  OrderedSeries (const SeriesName& name) : m_name(name)
  {

  }


  void init ()
  {
    static const std::size_t MinSize = 1000U; // TODO arbitrary

    m_times.reserve(MinSize);
    m_values.reserve(MinSize);
  }


public:
  
  OrderedSeries(const OrderedSeries&) = default;
  OrderedSeries(OrderedSeries&&) = default;


  static OrderedSeries create(const SeriesName& name)
  {
    OrderedSeries os{name};
    os.init();
    return os;
  }


  bool createIndex (const std::string& key) override
  {
    if (!isIndexed(key))
    {
      m_indexes.emplace(key, Index{});
      return true;
    }
    return false;
  }
  

  void add (const njson& times, njson&& values) override
  {
    const auto& timesVec = times.as<std::vector<SeriesTime>>();
    const auto& valuesVec = values.as<std::vector<SeriesValue>>();

    if (haveIndexes())
    {
      addIndexes(timesVec, valuesVec);
    }
    
    m_times.insert(m_times.cend(), timesVec.cbegin(), timesVec.cend());
    m_values.insert(m_values.cend(), std::make_move_iterator(valuesVec.begin()), std::make_move_iterator(valuesVec.end()));
  }


  njson get (const GetParams& params) const override
  {
    const auto [itStart, itEnd] = applyTimeRange(params);

    PLOGD << "Get from " << *itStart << " to " << (itEnd == m_times.cend() ? "end" : std::to_string(*itEnd));

    if (itStart == m_times.cend())
    {
      PLOGD << "No data";
      return njson (json_object_arg, {{"t", json_array_arg_t{}}, {"v", json_array_arg_t{}}});
    }
    else if (params.hasWhere())
    {
      PLOGD << "Applying where clause";
      return applyIndexes(params, itStart, itEnd);
    }
    else
    {
      return getData(itStart, itEnd);
    }
  }

  
private:

  void addIndexes(const std::vector<SeriesTime>& timesVec, const std::vector<SeriesValue>& valuesVec)
  {
    std::size_t index = 0;

    for (const auto& val : valuesVec)
    {
      for (const auto& member : val.object_range()) 
      {
        const auto& key = member.key();
        const auto& value = member.value();

        if (isIndexed(key))
        {
          if (const auto itIndex = m_indexes.at(key).index.find(value); itIndex != m_indexes.at(key).index.end())
            m_indexes.at(key).index.at(value).add(timesVec[index], index);
          else
            m_indexes.at(key).index.emplace(value, IndexNode{timesVec[index], index});
        }
      }
      ++index;
    }
  }


  bool haveIndexes () const
  {
    return !m_indexes.empty();
  }


  bool isIndexed (const std::string& key) const
  {
    return m_indexes.contains(key);
  }


  std::tuple<TimeVectorConstIt, TimeVectorConstIt> applyTimeRange (const GetParams& params) const
  {
    // if start not set, start is begin(), otherwise find start
    const auto itStart = params.isStartSet() ? std::lower_bound(m_times.cbegin(), m_times.cend(), params.getStart()) : m_times.cbegin();
    auto itEnd = params.isEndSet() ? std::upper_bound(itStart, m_times.cend(), params.getEnd()) : m_times.cend();
    return {itStart, itEnd};
  }


  njson applyIndexes (const GetParams& params, const TimeVectorConstIt itStart, const TimeVectorConstIt itEnd) const
  {
    njson rsp (json_object_arg, {{"t", json_array_arg_t{}}, {"v", json_array_arg_t{}}});

    const auto timeMin = *itStart;
    const auto timeMax = itEnd == m_times.cend() ? std::numeric_limits<SeriesTime>::max() : *itEnd;
    const auto condition = params.getWhere();

    for (const auto& member : condition.object_range())  // TODO check 'where' is an object
    {
      if (const auto& indexName = member.key() ; !isIndexed(indexName))
      {
        PLOGD << indexName << " not an index";        
      }
      else
      {
        PLOGD << indexName << " is indexed";

        // TODO need to sort data before it's returned: these functions should return the indexes, which are then sorted, then values are retrieved?
        // TODO need to handle multiple indexes in "where"

        for (const auto termMember : member.value().object_range())
        {
          const auto& term = termMember.key();
          const auto& indexMap = m_indexes.at(indexName).index;

          if (term == ">")
          {
            gt(params, timeMin, timeMax, indexMap, termMember.value(), rsp);
          }
          else if (term == ">=")
          {
            gte(params, timeMin, timeMax, indexMap, termMember.value(), rsp);
          }
          else if (term == "<")
          {
            lt(params, timeMin, timeMax, indexMap, termMember.value(), rsp);
          }
          else if (term == "<=")
          {
            lte(params, timeMin, timeMax, indexMap, termMember.value(), rsp);
          }
          else if (term == "==")
          {
            eq(params, timeMin, timeMax, indexMap, termMember.value(), rsp);
          }
          else if (term == "[]")
          {
            valueRangeInclusive(params, timeMin, timeMax, indexMap, termMember.value(), rsp);
          }
        }
      }
    }

    return rsp;
  }



  /*
  njson applyJsonPath (const WhereViewType& condition, const TimeVectorConstIt itStart, const TimeVectorConstIt itEnd) const
  {
    auto s = chrono::steady_clock::now();

    njson rsp (json_object_arg, {{"t", json_array_arg_t{}}, {"v", json_array_arg_t{}}});

    try
    {
      rsp["t"].reserve(500);
      rsp["v"].reserve(500); // TODO

      const auto end = std::distance(m_times.cbegin(), itEnd);

      for (auto i = std::distance(m_times.cbegin(), itStart); i < end ; ++i)
      {
        if (const auto whereResult = jsonpath::json_query(m_values[i], condition, jsonpath::result_options::path | jsonpath::result_options::nodups); !whereResult.empty())
          getData(i, rsp.at("t"), rsp.at("v"));
      }  
    }
    catch(const jsonpath::jsonpath_error& jpex)
    {
      PLOGE << jpex.what();
      rsp["t"].clear();
      rsp["v"].clear();
    }

    std::cout << "applyJsonPath(): " << chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - s).count() << '\n';

    return rsp;
  } */


  
  njson getData (const TimeVectorConstIt itStart, const TimeVectorConstIt itEnd) const 
  {
    const auto size = std::distance(itStart, itEnd);
    const auto start = std::distance(m_times.cbegin(), itStart);    
    const auto last = start + size;

    PLOGD << "Getting " << size << " time/values";

    njson rsp (json_object_arg, {{"t", json_array_arg_t{}}, {"v", json_array_arg_t{}}});

    rsp["t"].reserve(size);
    rsp["v"].reserve(size);

    for (auto source = start; source < last ; ++source)
      getData(source, rsp["t"], rsp["v"]);

    return rsp;
  }


  void getData (const std::ptrdiff_t index, njson& times, njson& values) const 
  {
    PLOGD << "Getting single time/value";
    
    times.push_back(m_times[index]);
    values.emplace_back(m_values[index]);
  }


  // operator functions
  void eq ( const GetParams& params, const SeriesTime& timeMin, const SeriesTime& timeMax, const std::map<njson, IndexNode>& indexMap,
            const njson& value, njson& rsp) const
  {
    PLOGD << "==";

    if (auto [first, end] = indexMap.equal_range(value) ; first != indexMap.cend())
    {
      const auto isFullTimeRange = params.isFullRange();
      while (first != end)
      {
        if (isFullTimeRange)
          getDataFromIndex(first->second.getTimes(), rsp);
        else
          getDataFromIndex(timeMin, timeMax, first->second.getTimes(), rsp);

        ++first;
      }
    }
  }


  void gt ( const GetParams& params, const SeriesTime& timeMin, const SeriesTime& timeMax, const std::map<njson, IndexNode>& indexMap,
            const njson& value, njson& rsp) const
  {
    PLOGD << ">";
    doGtGte(params, timeMin, timeMax, indexMap, rsp, [&indexMap, &value]()
    {
      return indexMap.upper_bound(value);
    });    
  }


  void gte (  const GetParams& params, const SeriesTime& timeMin, const SeriesTime& timeMax, const std::map<njson, IndexNode>& indexMap,
              const njson& value, njson& rsp) const
  {
    PLOGD << ">=";
    doGtGte(params, timeMin, timeMax, indexMap, rsp, [&indexMap, &value]()
    {
      return indexMap.lower_bound(value);
    });
  }


  void lt ( const GetParams& params, const SeriesTime& timeMin, const SeriesTime& timeMax, const std::map<njson, IndexNode>& indexMap,
            const njson& value, njson& rsp) const
  {
    PLOGD << "<";
    doLtLte(params, timeMin, timeMax, indexMap, rsp, [&indexMap, &value]()
    {
      return indexMap.lower_bound(value);
    });    
  }


  void lte (  const GetParams& params, const SeriesTime& timeMin, const SeriesTime& timeMax, const std::map<njson, IndexNode>& indexMap,
              const njson& value, njson& rsp) const
  {
    PLOGD << "<=";
    doLtLte(params, timeMin, timeMax, indexMap, rsp, [&indexMap, &value]()
    {
      return indexMap.upper_bound(value);
    });    
  }


  void doGtGte (const GetParams& params, const SeriesTime& timeMin, const SeriesTime& timeMax, const std::map<njson, IndexNode>& indexMap,
                njson& rsp, std::function<std::map<njson, IndexNode>::const_iterator (void)> getBounds) const
  {
    if (auto itIndex = getBounds(); itIndex != indexMap.cend())
    {
      const auto isFullTimeRange = params.isFullRange();

      while (itIndex != indexMap.cend())
      {
        if (isFullTimeRange)
          getDataFromIndex(itIndex->second.getTimes(), rsp);
        else
          getDataFromIndex(timeMin, timeMax, itIndex->second.getTimes(), rsp);

        ++itIndex;
      }
    }
  }


  void doLtLte (const GetParams& params, const SeriesTime& timeMin, const SeriesTime& timeMax, const std::map<njson, IndexNode>& indexMap,
                njson& rsp, std::function<std::map<njson, IndexNode>::const_iterator (void)> getBounds) const
  {
    if (const auto itEnd = getBounds(); itEnd != indexMap.cend())
    {
      const auto isFullTimeRange = params.isFullRange();
      
      for (auto itIndex = indexMap.cbegin(); itIndex != itEnd ; ++itIndex)
      {
        if (isFullTimeRange)
          getDataFromIndex(itIndex->second.getTimes(), rsp);
        else
          getDataFromIndex(timeMin, timeMax, itIndex->second.getTimes(), rsp);
      }
    }
  }

  
  void valueRangeInclusive (const GetParams& params, const SeriesTime& timeMin, const SeriesTime& timeMax, const std::map<njson, IndexNode>& indexMap, const njson& range, njson& rsp) const
  { 
    const auto& lowValue = range[0];
    const auto& highValue = range[1];

    PLOGD << "Value Range: " << range;

    if (lowValue < highValue)
    {
      if (const auto itRangeLow = indexMap.lower_bound(lowValue); itRangeLow != indexMap.cend())
      {
        if (const auto itRangeHigh = indexMap.upper_bound(highValue); itRangeHigh != indexMap.cend())
        {
          const auto isFullTimeRange = params.isFullRange();
          for(auto itIndex = itRangeLow ; itIndex != itRangeHigh ; ++itIndex)
          {
            if (isFullTimeRange)
              getDataFromIndex(itIndex->second.getTimes(), rsp);
            else
              getDataFromIndex(timeMin, timeMax, itIndex->second.getTimes(), rsp);
          }
        }
      }
    }
  }


  void getDataFromIndex (const SeriesTime& timeMin, const SeriesTime& timeMax, const IndexNode::IndexedTimes& times, njson& rsp) const
  {
    for (const auto& [time, index] : times)
    {
      if (time >= timeMin && time < timeMax)
      {
        PLOGD << "Found time " << time;
        getData(index, rsp["t"], rsp["v"]);
      }
    }
  }


  void getDataFromIndex (const IndexNode::IndexedTimes& times, njson& rsp) const
  {
    for (const auto& [time, index] : times)
    {
      PLOGD << "Found time " << time;
      getData(index, rsp["t"], rsp["v"]);
    }
  }


private:
  SeriesName m_name;
  TimeVector m_times;
  ValueVector m_values;
  Indexes m_indexes;
};


} // ns ts
} // ns core
} // ns nemesis

#endif
