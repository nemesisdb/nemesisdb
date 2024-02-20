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
    const auto [itStart, itEnd] = applyRange(params);

    if (itStart == m_times.cend())
    {
      PLOGD << "No data";
      return njson (json_object_arg, {{"t", json_array_arg_t{}}, {"v", json_array_arg_t{}}});
    }
    else if (params.hasWhere())
    {
      PLOGD << "Applying where clause";
      // return applyFilter(params.getWhere(), itStart, itEnd);
      return applyIndexes(params.getWhere(), itStart, itEnd);
    }
    else
    {
      PLOGD << "Get from " << *itStart << " to " << (itEnd == m_times.cend() ? "end" : std::to_string(*itEnd));
      return getData(itStart, itEnd);
    }
  }

  
private:

  void addIndexes(const std::vector<SeriesTime>& timesVec, const std::vector<SeriesValue>& valuesVec)
  {
    std::size_t index = 0;

    for (const auto& val : valuesVec)
    {
      for (const auto& member : val.object_range()) // TODO assumes object
      {
        const auto& key = member.key();
        const auto& value = member.value();

        if (isIndexed(key))
        {
          if (const auto itIndex = m_indexes.at(key).index.find(value); itIndex != m_indexes.at(key).index.end())
            m_indexes.at(key).index.at(value).times.emplace_back(timesVec[index]);
          else
            m_indexes.at(key).index.emplace(value, IndexNode{.times = std::vector<SeriesTime>{timesVec[index]}});
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


  std::tuple<TimeVectorConstIt, TimeVectorConstIt> applyRange (const GetParams& params) const
  {
    //auto s = chrono::steady_clock::now();

    // if start not set, start is begin(), otherwise find start
    const auto itStart = params.isStartSet() ? std::lower_bound(m_times.cbegin(), m_times.cend(), params.getStart()) : m_times.cbegin();
    const auto itEnd = params.isEndSet() ? std::upper_bound(itStart, m_times.cend(), params.getEnd()) : m_times.cend();

    //std::cout << "applyRange(): " << chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - s).count() << '\n';

    return {itStart, itEnd};
  }


  njson applyIndexes (const WhereType& condition, const TimeVectorConstIt itStart, const TimeVectorConstIt itEnd) const
  {
    njson rsp (json_object_arg, {{"t", json_array_arg_t{}}, {"v", json_array_arg_t{}}});

    const auto timeMin = *itStart;
    const auto timeMax = *itEnd;

    // TODO tidy this mess
    for (const auto& member : condition.object_range())  // TODO check 'where' is an object
    {
      if (const auto& indexName = member.key(); isIndexed(indexName))
      {
        PLOGD << indexName << " is indexed";

        for (const auto termMember : member.value().object_range())
        {
          const auto& term = termMember.key();

          if (term == ">")
          {
            const auto& indexMap = m_indexes.at(indexName).index;

            PLOGD << ">";
            if (auto itIndex = indexMap.upper_bound(termMember.value()); itIndex != indexMap.cend())
            {
              while (itIndex != indexMap.cend())
              {
                for (const auto& time : itIndex->second.times)
                {
                  if (time >= timeMin && time < timeMax)
                  {
                    PLOGD << "Found " << time;
                    getData(time, rsp["t"], rsp["v"]);
                  }
                }
                ++itIndex;
              }
            }
          }
        }
      }
    }

    return rsp;
  }



  // njson applyFilter (const WhereViewType& condition, const TimeVectorConstIt itStart, const TimeVectorConstIt itEnd) const
  // {
  //   auto s = chrono::steady_clock::now();

  //   njson rsp (json_object_arg, {{"t", json_array_arg_t{}}, {"v", json_array_arg_t{}}});

  //   try
  //   {
  //     rsp["t"].reserve(500);
  //     rsp["v"].reserve(500); // TODO

  //     const auto end = std::distance(m_times.cbegin(), itEnd);

  //     for (auto i = std::distance(m_times.cbegin(), itStart); i < end ; ++i)
  //     {
  //       if (const auto whereResult = jsonpath::json_query(m_values[i], condition, jsonpath::result_options::path | jsonpath::result_options::nodups); !whereResult.empty())
  //         getData(i, rsp.at("t"), rsp.at("v"));
  //     }  
  //   }
  //   catch(const jsonpath::jsonpath_error& jpex)
  //   {
  //     PLOGE << jpex.what();
  //     rsp["t"].clear();
  //     rsp["v"].clear();
  //   }

  //   std::cout << "applyFilter(): " << chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now() - s).count() << '\n';

  //   return rsp;
  // }

  
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
