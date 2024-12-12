#ifndef NDB_CORE_ARRARRAY_H
#define NDB_CORE_ARRARRAY_H

#include <algorithm>
#include <core/NemesisCommon.h>
#include <core/arr/ArrCommands.h>
#include <core/arr/ArrArray.h>


namespace nemesis { namespace arr {


using std::vector;


/*
A fixed-sized std::vector. Does not perform bounds checks, but
does provide functions for that purpose.
*/
template<typename T, bool Sorted>
class Array
{
public:

  using ValueT = T;
  
  Array(const std::size_t size) : m_size(size), m_used(0)
  {
    m_array.resize(m_size);
  }


  static bool isRequestedSizeValid(const std::size_t size) noexcept
  {
    return size == std::clamp<std::size_t>(size, 1, 1'000); // TODO quite arbitrary
  }


  bool isInBounds(const std::size_t pos) const noexcept
  {
    return pos < m_size;
  }


  bool isFull() const noexcept
  {
    return m_used >= m_size;
  }


  bool hasCapacity(const std::size_t n) const noexcept
  {
    return m_used+n <= m_size;
  }


  bool isSetInBounds(const std::size_t start, const std::size_t setSize) const noexcept requires (!Sorted)
  {
    return start < m_size && start + setSize <= m_size;
  }


  void set(const std::size_t pos, const T& item) requires (!Sorted)
  {
    m_array[pos] = item;
  }


  void set(const T& item)
  {
    m_array[m_used++] = item;

    if constexpr (Sorted)
      std::sort(m_array.begin(), std::next(m_array.begin(), m_used));
  }


  void setRange(std::size_t pos, const std::vector<T>& items) requires (!Sorted)
  { 
    for (const auto& item : items)
      m_array[pos++] = item;

    m_used += items.size();
  }

  
  void setRange(const std::vector<T>& items)
  {    
    for (const auto& item : items)
      m_array[m_used++] = item;

    if constexpr (Sorted)
      std::sort(m_array.begin(), std::next(m_array.begin(), m_used));
  }


  T get(const std::size_t pos) const
  {
    return m_array[pos];
  }


  njson getRange(const std::size_t start) const requires (!Sorted)
  {
    return getRange(start, m_size);
  }


  njson getRange(const std::size_t start) const requires (Sorted)
  {
    return getRange(start, m_used);
  }


  njson getRange(const std::size_t start, std::size_t stop) const
  {
    if constexpr (Sorted)
      stop = std::min<std::size_t> (stop, m_used);
    else
      stop = std::min<std::size_t> (stop, m_size);  

    const auto itStart = std::next(m_array.cbegin(), start);
    const auto itEnd = std::next(m_array.cbegin(), stop);
    const auto rangeSize = std::distance(itStart, itEnd);

    PLOGD << "Array::getRange(): " << start << " to " << stop;

    njson rsp{njson::make_array(std::min<std::size_t>(rangeSize, 100))};
    
    std::size_t i = 0;
    std::for_each(itStart, itEnd, [&rsp, &i](const auto& item)
    {
      rsp[i++] = item;
    });

    return rsp;
  }


  void swap(const std::size_t posA, const std::size_t posB) requires (!Sorted)
  {
    std::iter_swap( std::next(m_array.begin(), posA),
                    std::next(m_array.begin(), posB));
  }


  void clear(const std::size_t start)
  {
    clear(start, m_size);
    m_used = 0;
  }


  void clear(const std::size_t start, const std::size_t stop) requires (!Sorted)
  {
    PLOGD << "Array::clear(): " << start << " to " << stop;

    const auto itStart = std::next(m_array.begin(), start);
    const auto itEnd = std::next(m_array.begin(), std::min<std::size_t>(m_size, stop));

    for (auto it = itStart ; it != itEnd; ++it)
      *it = T{};

    m_used -= stop-start;
  }


  void clear(const std::size_t start, const std::size_t stop) requires (Sorted)
  {
    PLOGD << "Array::clear(): " << start << " to " << stop;

    const auto itStart = std::next(m_array.begin(), start);
    const auto itPivot = std::next(m_array.begin(), std::min<std::size_t>(m_used, stop));

    // don't rotate if it's the entire array    
    if (!(itStart == m_array.begin() && itPivot == m_array.end()))
    {
      std::rotate(itStart, itPivot, m_array.end());
      m_used -= std::distance(itStart, itPivot);
    }
    else
    {
      m_used = 0;
    }
  }


  std::size_t size() const noexcept
  {
    return m_size;
  }


  std::vector<T>::const_iterator cbegin() const noexcept requires (Sorted)
  {
    return m_array.cbegin();
  }


  std::vector<T>::const_iterator cend() const noexcept requires (Sorted)
  {
    return std::next(m_array.cbegin(), m_used);
  }

private:
  std::vector<T> m_array;
  std::size_t m_size;
  std::size_t m_used;
};


using OArray = Array<njson, false>;
using IArray = Array<std::int64_t, false>;
using SArray = Array<std::string, false>;

using SortedIArray = Array<std::int64_t, true>;

}
}

#endif
