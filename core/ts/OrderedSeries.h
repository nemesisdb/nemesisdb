#ifndef NDB_CORE_TSORDEREDSERIES_H
#define NDB_CORE_TSORDEREDSERIES_H

#include <vector>
#include <map>
#include <queue> // for priority_queue
#include <core/ts/TsCommon.h>


namespace nemesis { namespace core { namespace ts {


using namespace jsoncons;
using namespace jsoncons::literals;


/// Represents a time series that submitted data is always gauranteed to be ordered by the sender. 
/// Uses parallel vectors to hold time and value data:
///   An event occurring at m_times[i] has metrics in m_values[i].
class OrderedSeries : public BaseSeries
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
    
    bool leave = false;
    std::vector<std::tuple<Index::IndexMapConstIt, Index::IndexMapConstIt>> indexRanges;

    for (const auto& member : condition.object_range())  // TODO check 'where' is an object
    {
      if (const auto& indexName = member.key() ; !isIndexed(indexName))
      {
        PLOGD << indexName << " not an index";
        leave = true;
      }
      else
      {
        PLOGD << indexName << " is indexed";

        // TODO need to handle multiple indexes in "where"
        
        for (const auto termMember : member.value().object_range())
        {
          const auto& op = termMember.key();
          const auto& condition = termMember.value();
          const auto& indexMap = m_indexes.at(indexName).index;
          std::tuple<Index::IndexMapConstIt,Index::IndexMapConstIt> indexRange; // start, end of indexes matching criteria

          if (op == "<")
            indexRange = lt(indexMap, condition, rsp);
          else if (op == "<=")
            indexRange = lte(indexMap, condition, rsp);
          else if (op == ">")
            indexRange = gt(indexMap, condition, rsp);
          else if (op == ">=")
            indexRange = gte(indexMap, condition, rsp);
          else if (op == "==")
            indexRange = eq(indexMap, condition, rsp);
          else if (op == "[]")
            indexRange = rng(indexMap, condition, rsp);


          if (const auto [start, end] = indexRange; start == indexMap.cend())
          {
            leave = true;  // terms are &&, i.e. if (term1 && term2), so if a term returns no results, can bail
            break;
          }
          else
            indexRanges.emplace_back(start, end);            
        }

        if (leave)
          break;
      }
    }


    if (!leave && !indexRanges.empty())
    {
      const auto result = intersectIndexResults(indexRanges, timeMin, timeMax);
      
      for (const auto& [time, index] : result)
        getData(index, rsp["t"], rsp["v"]);
    }


    return rsp;
  }


  std::vector<std::tuple<SeriesTime, std::size_t>> intersectIndexResults (const std::vector<std::tuple<Index::IndexMapConstIt, Index::IndexMapConstIt>>& allResults, const SeriesTime timeMin, const SeriesTime timeMax) const
  {
    // TODO don't think this is correct: this discards subsequent results unless index is not present in all, but it may be present in just one:
    //  if (temp < 10 && sensor < 3)
    // temp: 4, sensor: 1
    // is valid, but current impl will dismiss this because it's not present in all sensor index nodes
    // I think.

    // something like:
    //  intersectedResult = allResults[0] (within given time limits)
    //  for each intersected
    //    for each allResult
    //      if intersected index in allResult[i] AND time within range
    //        do nothing
    //      else
    //        erase from intersectedResult

    auto sort = [](const std::tuple<SeriesTime, std::size_t>& a, const std::tuple<SeriesTime, std::size_t>& b)
    {
      return std::get<1>(a) < std::get<1>(b); // TODO possibly check time here 
    };


    auto inIndexRange = [sort](const Index::IndexMapConstIt& itStart, const Index::IndexMapConstIt& itEnd, const std::tuple<SeriesTime, std::size_t>& timeToIndex)
    {
      for (auto it = itStart ; it != itEnd ; ++it)
      {
        // TODO wrong , discarding whole range rather than individual indexes within each node
        if (!std::binary_search(std::cbegin(it->second.getTimes()), std::cend(it->second.getTimes()), timeToIndex, sort))
          return false;
      }
      return true;
    };


    std::vector<std::tuple<SeriesTime, std::size_t>> intersectedResult;

    const auto [start, end] = allResults[0];
    std::for_each(start, end, [&intersectedResult, timeMin, timeMax](const auto pair)
    {
      const auto& times = pair.second.getTimes();
      std::for_each (std::cbegin(times), std::cend(times), [&intersectedResult, timeMin, timeMax](const auto timeToIndex)
      {
        if (const auto& [time, index] = timeToIndex; time >= timeMin && time < timeMax)
          intersectedResult.emplace_back(time, index);
      });
    });


    std::sort(std::begin(intersectedResult), std::end(intersectedResult), sort);

    if (allResults.size() > 1)
    {
      for (auto itIntersected = intersectedResult.begin() ; itIntersected != intersectedResult.end() ; )
      {
        for (std::size_t i = 1 ; i < allResults.size() ; ++i)
        {
          const auto& [start, end] = allResults[i];
          
          if (!inIndexRange(start, end, *itIntersected))
            itIntersected = intersectedResult.erase (itIntersected);
          else
            ++itIntersected;
        }
      }
    }
    

    return intersectedResult;
  }

  
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



  void getData (const std::size_t index, njson& times, njson& values) const 
  {
    PLOGD << "Getting single time/value";
    
    times.push_back(m_times[index]);
    values.emplace_back(m_values[index]);
  }


  // operator functions:
  //  each returns an iterator to the start and end to Index entries which matched the criteria
  //  these are returned to caller, then sorted by the time index, before retrieving values and returning the response.

  std::tuple<Index::IndexMapConstIt, Index::IndexMapConstIt> eq (const std::map<njson, IndexNode>& indexMap, const njson& value, njson& rsp) const
  {
    PLOGD << "==";
    
    if (const auto [first, end] = indexMap.equal_range(value); first == indexMap.cend())
      return {indexMap.cend(), indexMap.cend()};
    else
      return {first, end};
  }


  std::tuple<Index::IndexMapConstIt, Index::IndexMapConstIt> lt (const std::map<njson, IndexNode>& indexMap, const njson& value, njson& rsp) const
  {
    PLOGD << "<";
    return {indexMap.cbegin(), indexMap.lower_bound(value)}; 
  }


  std::tuple<Index::IndexMapConstIt, Index::IndexMapConstIt> lte (const std::map<njson, IndexNode>& indexMap, const njson& value, njson& rsp) const
  {
    PLOGD << "<=";
    return {indexMap.cbegin(), indexMap.upper_bound(value)}; 
  }


  std::tuple<Index::IndexMapConstIt, Index::IndexMapConstIt> gt (const std::map<njson, IndexNode>& indexMap, const njson& value, njson& rsp) const
  {
    PLOGD << ">";
    return {indexMap.upper_bound(value), indexMap.cend()};
  }


  std::tuple<Index::IndexMapConstIt, Index::IndexMapConstIt> gte (const std::map<njson, IndexNode>& indexMap, const njson& value, njson& rsp) const
  {
    PLOGD << ">=";
    return {indexMap.lower_bound(value), indexMap.cend()};
  }


  std::tuple<Index::IndexMapConstIt, Index::IndexMapConstIt> rng (const std::map<njson, IndexNode>& indexMap, const njson& range, njson& rsp) const
  {
    PLOGD << "[]";

    const auto& low = range[0];
    const auto& high = range[1];

    if (low < high) // TODO check this in TsHandler
    {
      if (const auto itRangeLow = indexMap.lower_bound(low); itRangeLow != indexMap.cend())
        return {itRangeLow, indexMap.upper_bound(high)};
    }

    return {indexMap.cend(), indexMap.cend()};
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
