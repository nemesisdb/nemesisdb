#ifndef NDB_CORE_LSTLIST_H
#define NDB_CORE_LSTLIST_H

#include <list>
#include <tuple>
#include <vector>
#include <ranges>
#include <core/NemesisCommon.h>


namespace nemesis { namespace lst {

  template<typename T>
  class List
  {
  public:
    using ValueT = T;

  private:
    using It = std::list<T>::iterator;
    using ConstIt = std::list<T>::const_iterator;

  public:

    bool isInbounds(const std::size_t pos) const noexcept
    {
      return pos < m_list.size();
    }


    std::size_t size() const noexcept
    {
      return m_list.size();
    }


    std::tuple<std::size_t, std::size_t> addHead(const njson& items)
    {
      return add(items);
    }


    std::tuple<std::size_t, std::size_t> addTail(const njson& items)
    {
      return add(items, m_list.size());
    }

    std::tuple<std::size_t, std::size_t> add(const njson& items, const std::size_t pos = 0)
    {
      const auto& itemsVector = items.as<std::vector<T>>(); // TODO this may be expensive, use array_range()
      const auto itItems = itemsVector.cbegin();
      const auto itItemsEnd = itemsVector.cend();
      const auto itPos = std::next(m_list.cbegin(), std::min<std::size_t>(pos, m_list.size()));
      
      return doAdd(itPos, itItems, itItemsEnd);
    }


    void setRange(const njson& items, const std::size_t start)
    {
      auto itStart = std::next(m_list.begin(), start);
      const auto itEnd = std::next(itStart, items.size());

      auto itemsIt = items.array_range().cbegin();
      std::ranges::subrange range{itStart, itEnd};

      for (auto& item : range)
      {
        item = *itemsIt;
        itemsIt = std::next(itemsIt);
      }
    }

    
    T get(const std::size_t pos) const
    {
      const auto it = std::next(m_list.cbegin(), pos);
      return *it;
    }


    std::vector<T> getRange(const std::size_t start) const
    {
      return getRange(start, m_list.size());
    }


    std::vector<T> getRange(const std::size_t start, const std::size_t end) const
    {
      const auto itStart = std::next(m_list.cbegin(), start);
      const auto itEnd = std::next(m_list.cbegin(), std::min<std::size_t>(end, m_list.size()));

      std::vector<T> result;
      result.reserve(std::min<std::size_t>(std::distance(itStart, itEnd), 1000)); // TODO MaxResponseSize

      std::for_each(itStart, itEnd, [&result](const auto item)
      {
        result.emplace_back(item);
      });

      return result;
    }

  private:
    // Returns (pos, size)
    std::tuple<std::size_t, std::size_t> doAdd(const ConstIt dstIt, const std::vector<T>::const_iterator srcIt,
                                                                    const std::vector<T>::const_iterator srcItEnd)
    {
      const auto insertedIt = m_list.insert(dstIt, srcIt, srcItEnd);
      return {std::distance(m_list.begin(), insertedIt), m_list.size()};
    }

  private:
    std::list<T> m_list;
  };


  using ObjectList = List<njson>;

}
}

#endif
