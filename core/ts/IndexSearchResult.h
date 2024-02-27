#ifndef NDB_CORE_TSINDEXSEARCHRESULTS_H
#define NDB_CORE_TSINDEXSEARCHRESULTS_H

#include <vector>
#include <map>
#include <queue> // for priority_queue
#include <core/ts/TsCommon.h>


namespace nemesis { namespace core { namespace ts {

using namespace jsoncons;
using namespace nemesis::core::ts;


class IndexSearchResult
{
  using IndexMapConstIt = Index::IndexMapConstIt;

public:
  IndexSearchResult() = default;


  void addResult (const std::string& indexName, const std::tuple<IndexMapConstIt, IndexMapConstIt>& result)
  {
    m_results[indexName].emplace_back(result);
  }


  void intersect (const SeriesTime& timeMin, const SeriesTime& timeMax)
  {
    if (!haveResults())
      return ;

    // populate with the first result
    PLOGD << "Intersect initialising for: " << m_results.cbegin()->first;

    for (const auto& [start, end] : m_results.cbegin()->second)
    {
      std::for_each(start, end, [this, timeMin, timeMax](const auto& index) mutable
      {
        const auto& times = index.second.getTimes();
        for (const auto [time, index] : times)
        {
          if (time >= timeMin && time < timeMax)
            m_intersectedResult.emplace_back(time, index);
        }
      });
    }
    
    if (!m_intersectedResult.empty())
    {
      std::sort(std::begin(m_intersectedResult), std::end(m_intersectedResult), IndexNode::IndexComparer{});

      // for remaining terms, only retain the indexes that are in result and in the term (and within time limits)
      for (auto itTerm = std::next(m_results.cbegin()) ; itTerm != m_results.cend() ; ++itTerm)
      {
        PLOGD << "Intersecting for: " << itTerm->first ;
        intersectTermResults(itTerm->second, timeMin, timeMax);

        if (m_intersectedResult.empty()) 
          break;
      }
    }
  }


  bool haveResults () const
  {
    return !m_results.empty();
  }

  
  const IndexNode::IndexedTimes& result () const
  {
    return m_intersectedResult;
  }


private:
  void intersectTermResults (const std::vector<std::tuple<IndexMapConstIt, IndexMapConstIt>>& termResults, const SeriesTime& timeMin, const SeriesTime& timeMax)
  {
    IndexNode::IndexedTimes intersected;

    dumpIndexResult("m_intersectedResult:", m_intersectedResult);

    for (const auto& sourceTimeIndex : m_intersectedResult)
    {
      bool found = false;

      PLOGD << "Looking for [" << std::get<0>(sourceTimeIndex) << ',' << std::get<1>(sourceTimeIndex) << ']';
      
      for (const auto& [start, end] : termResults)  // for each indexed search result
      {
        for (auto indexNode = start ; indexNode != end ; ++indexNode) // for each node
        {
          const auto& times = indexNode->second.getTimes();

          dumpIndexResult("in:", times);

          if (std::binary_search(std::cbegin(times), std::cend(times), sourceTimeIndex, IndexNode::IndexTimeComparer{timeMin, timeMax}))
          {
            found = true;
            intersected.emplace_back(std::get<0>(sourceTimeIndex), std::get<1>(sourceTimeIndex));
          }
          
          if (found)  // the same index cannot be found twice in the same search (i.e. "temp" can only be one temperate at a given time)
            break;
        }        
      }

      dumpIndexResult("Result: ", intersected);
    }

    m_intersectedResult = std::move(intersected);
  }


  
  void dumpIndexResult (const std::string_view msg, const IndexNode::IndexedTimes& times) const
  {
    #ifdef NDB_DEBUG
    PLOGD << msg;

    std::stringstream ss;

    for (const auto& [t,i] : times)
      ss << '[' << t << ',' << i << ']';
    
    PLOGD << ss.str();
    #endif
  }
  


private:
  std::map<std::string, std::vector<std::tuple<IndexMapConstIt, IndexMapConstIt>>> m_results;
  IndexNode::IndexedTimes m_intersectedResult;
};


} // ns ts
} // ns core
} // ns nemesis

#endif
