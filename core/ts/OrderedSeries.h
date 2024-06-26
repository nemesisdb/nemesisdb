#ifndef NDB_CORE_TSORDEREDSERIES_H
#define NDB_CORE_TSORDEREDSERIES_H

#include <vector>
#include <map>
#include <core/ts/TsCommon.h>
#include <core/ts/IndexSearchResult.h>


namespace nemesis { namespace core { namespace ts {


using namespace jsoncons;
using namespace jsoncons::literals;

inline static const njson EmptyResult (json_object_arg, {{"t", json_array_arg_t{}}, {"evt", json_array_arg_t{}}});


/// Represents a time series where submitted data is always gauranteed to be ordered by the sender.
/// Or in other words: OrderedSeries does not sort the data before it is stored. 
/// Uses parallel vectors to hold time and value data:
///   An event occurring at m_times[i] has event object at m_events[i].
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
    m_events.reserve(MinSize);
  }


public:
  
  OrderedSeries(const OrderedSeries&) = default;
  OrderedSeries(OrderedSeries&&) = default;


  static OrderedSeries create(const SeriesName& name)
  {
    PLOGD << "Creating OrderedSeries " << name;

    OrderedSeries os{name};
    os.init();
    return os;
  }


  bool createIndex (const std::string& key) override
  {
    if (!isIndexed(key))
    {
      buildIndex(key);
      return true;
    }
    return false;
  }
  

  void add (const njson& times, njson&& events) override
  {
    const auto& timesVec = times.as<std::vector<SeriesTime>>();
    const auto& eventsVec = events.as<std::vector<SeriesValue>>();

    // should always be checked by TsHandler, but it's too important to not assert here
    assert(timesVec.size() == eventsVec.size());

    if (haveIndexes())
      addIndexes(timesVec, eventsVec);

    m_times.insert(m_times.cend(), timesVec.cbegin(), timesVec.cend());  
    m_events.insert(m_events.cend(), std::make_move_iterator(eventsVec.begin()), std::make_move_iterator(eventsVec.end()));
  }


  std::tuple<TsRequestStatus,njson> get (const GetParams& params) const override
  {
    const auto [itStart, itEnd] = applyTimeRange(params);

    PLOGD << "Get from " << *itStart << " to " << (itEnd == m_times.cend() ? "end" : std::to_string(*itEnd));

    if (itStart == m_times.cend())
    {
      PLOGD << "No data";
      return {TsRequestStatus::Ok, EmptyResult};
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


  // Indexes existing data
  void buildIndex (const std::string& key)
  {
    m_indexes.emplace(key, Index{});

    for (std::size_t pos = 0 ; pos < m_events.size() ; ++pos)
    {
      // if event contains the key (event is always a JSON object and only top level members can be indexed)
      if (const auto& evt = m_events[pos]; evt.contains(key))
      {
        const auto& evtValue = evt.at(key);

        // create/add to index
        if (const auto itIndex = m_indexes.at(key).index.find(evtValue); itIndex != m_indexes.at(key).index.end())
          itIndex->second.add(m_times[pos], pos);
        else
          m_indexes.at(key).index.emplace(evtValue, IndexNode{m_times[pos], pos});
      }
    }
  }


  // Index data received by add()
  void addIndexes(const std::vector<SeriesTime>& times, const std::vector<SeriesValue>& events)
  {
    for (std::size_t i = 0, pos = m_times.size() ; i < events.size() ; ++i, ++pos)
    {
      for (const auto& member : events[i].object_range())
      {
        if (const auto& key = member.key(); isIndexed(key))
        {
          const auto& value = member.value();

          if (const auto itIndex = m_indexes.at(key).index.find(value); itIndex != m_indexes.at(key).index.end())
            itIndex->second.add(times[i], pos);
          else
            m_indexes.at(key).index.emplace(value, IndexNode{times[i], pos});
        }
      }
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
    const auto itEnd = params.isEndSet() ? std::upper_bound(itStart, m_times.cend(), params.getEnd()) : m_times.cend();
    return {itStart, itEnd};
  }


  std::tuple<TsRequestStatus,njson> applyIndexes (const GetParams& params, const TimeVectorConstIt itStart, const TimeVectorConstIt itEnd) const
  {
    njson rsp (json_object_arg, {{"t", json_array_arg_t{}}, {"evt", json_array_arg_t{}}});
    TsRequestStatus status = TsRequestStatus::Ok;

    const auto timeMin = *itStart;
    const auto timeMax = itEnd == m_times.cend() ? std::numeric_limits<SeriesTime>::max() : *itEnd;
    const auto& condition = params.getWhere();
    
    bool emptyResult = false;
    std::vector<std::vector<std::tuple<SeriesTime, std::size_t>>> indexes;
    IndexSearchResult result;

        
    for (const auto& termObject : condition.object_range())
    {
      if (const auto& indexName = termObject.key() ; !isIndexed(indexName))
      {
        PLOGE << indexName << " not an index";
        emptyResult = true;
        status = TsRequestStatus::NotIndexed;
      }
      else
      {
        PLOGD << indexName << " is indexed";

        assert(termObject.value().size() == 1U);

        const auto& conditionObject = *(termObject.value().object_range().cbegin());

        const auto& op = conditionObject.key(); // operator
        const auto& operand = conditionObject.value();
        const auto& indexMap = m_indexes.at(indexName).index;
        std::tuple<Index::IndexMapConstIt,Index::IndexMapConstIt> indexRange; // [start, end] of index nodes matching criteria

        if (op == "<")
          indexRange = lt(indexMap, operand, rsp);
        else if (op == "<=")
          indexRange = lte(indexMap, operand, rsp);
        else if (op == ">")
          indexRange = gt(indexMap, operand, rsp);
        else if (op == ">=")
          indexRange = gte(indexMap, operand, rsp);
        else if (op == "==")
          indexRange = eq(indexMap, operand, rsp);
        else if (op == "[]")
          indexRange = rng(indexMap, operand, rsp);


        if (const auto [start, end] = indexRange; start == indexMap.cend())
        {
          emptyResult = true;  // terms are &&, i.e. if (term1 && term2), so if a term returns no results, can bail
          break;
        }
        else
        {
          result.addResult(indexName, indexRange);
        }

        if (emptyResult)
          break;
      }
    }


    if (!emptyResult && result.haveResults())
    {
      result.intersect(timeMin, timeMax);

      const auto& intersectedResult = result.result();
      
      rsp["t"].reserve(intersectedResult.size());
      rsp["evt"].reserve(intersectedResult.size());

      for (const auto& [time, index] : intersectedResult)
        getData(index, rsp["t"], rsp["evt"]);
    }


    return {status, rsp};
  }
  
  
  std::tuple<TsRequestStatus,njson> getData (const TimeVectorConstIt itStart, const TimeVectorConstIt itEnd) const 
  {
    const auto size = std::distance(itStart, itEnd);
    const auto start = std::distance(m_times.cbegin(), itStart);    
    const auto last = start + size;

    PLOGD << "Getting " << size << " time/values";

    njson rsp (json_object_arg, {{"t", json_array_arg_t{}}, {"evt", json_array_arg_t{}}});

    rsp["t"].reserve(size);
    rsp["evt"].reserve(size);

    for (auto source = start; source < last ; ++source)
      getData(source, rsp["t"], rsp["evt"]);

    return {TsRequestStatus::Ok, rsp};
  }



  void getData (const std::size_t index, njson& times, njson& events) const 
  {
    PLOGD << "Getting single time/value";
    
    times.push_back(m_times[index]);
    events.emplace_back(m_events[index]);
  }


  // operator functions:
  //  each returns an iterator to the start and end to Index entries which matched the criteria
  //  these are added to the results and intersected by IndexSearchResults

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

    if (low < high) // TODO check in TsHandler
    {
      if (const auto itRangeLow = indexMap.lower_bound(low); itRangeLow != indexMap.cend())
        return {itRangeLow, indexMap.upper_bound(high)};
    }

    return {indexMap.cend(), indexMap.cend()};
  }


private:
  SeriesName m_name;
  TimeVector m_times;
  ValueVector m_events;
  Indexes m_indexes;
};


} // ns ts
} // ns core
} // ns nemesis

#endif
