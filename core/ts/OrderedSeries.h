#ifndef NDB_CORE_TSORDEREDSERIES_H
#define NDB_CORE_TSORDEREDSERIES_H

#include <vector>
#include <core/ts/TsCommon.h>


namespace nemesis { namespace core { namespace ts {


using namespace jsoncons;
using namespace jsoncons::literals;


/// Represents a time series that is always gauranteed to be ordered by the sender. 
/// Uses parallel vectors to hold time and value data:
///   An event occurring at m_times[i] has metrics in m_values[i].
class OrderedSeries : public BasicSeries
{
  using TimeVector = std::vector<SeriesTime>;
  using ValueVector = std::vector<SeriesValue>;
  using TimeVectorConstIt = TimeVector::const_iterator;


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
  

  void add (const SeriesTime td, const SeriesValue value) override
  {
    m_times.push_back(td);
    m_values.push_back(std::move(value));
  }


  void add (const njson& times, njson&& values) override
  {
    const auto& timesVec = times.as<std::vector<SeriesTime>>();
    const auto& valuesVec = values.as<std::vector<SeriesValue>>();

    m_times.insert(m_times.cend(), timesVec.cbegin(), timesVec.cend());
    m_values.insert(m_values.cend(), std::make_move_iterator(valuesVec.begin()), std::make_move_iterator(valuesVec.end()));
  }


  njson get (const GetParams& params) override
  {
    // if start not set, start is begin(), otherwise find start
    const auto itStart = params.start == 0 ? m_times.cbegin() : std::lower_bound(m_times.cbegin(), m_times.cend(), params.start) ;
    const auto itEnd = params.end == 0 ? m_times.cend() : std::upper_bound(itStart, m_times.cend(), params.end);

    if (itStart == m_times.cend())
    {
      PLOGD << "No data";
      return njson (json_object_arg, {{"t", json_array_arg_t{}}, {"v", json_array_arg_t{}}});
    }
    else  [[likely]]
    {
      PLOGD << "Get from " << *itStart << " to " << (itEnd == m_times.cend() ? "end" : std::to_string(*itEnd));
      return getData(itStart, itEnd);
    }
  }


private:

  
  njson getData (const TimeVectorConstIt itStart, const TimeVectorConstIt itEnd)
  {
    const auto size = std::distance(itStart, itEnd);
    const auto start = std::distance(m_times.cbegin(), itStart);    
    const auto last = start + size;

    PLOGD << "Getting " << size << " time/values";

    njson rsp (json_object_arg, {{"t", json_array_arg_t{}}, {"v", json_array_arg_t{}}});

    rsp["t"].reserve(size);
    rsp["v"].reserve(size);

    for (auto source = start; source < last ; ++source)
    {
      rsp["t"].push_back(m_times[source]);
      rsp["v"].emplace_back(m_values[source]);
    }

    return rsp;
  }


private:
  SeriesName m_name;
  TimeVector m_times;
  ValueVector m_values;
};


} // ns ts
} // ns core
} // ns nemesis

#endif
