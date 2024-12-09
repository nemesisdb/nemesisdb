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
template<typename T>
class Array
{
public:

  using ValueT = T;
  
  Array(const std::size_t size) : m_size(size)
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


  bool isSetInBounds(const std::size_t start, const std::size_t setSize) const noexcept
  {
    return start < m_size && start + setSize <= m_size;
  }


  void set(const std::size_t pos, const T& item)
  {
    m_array[pos] = item;
  }


  void setRange(std::size_t pos, const std::vector<T>& items)
  {    
    for (const auto& item : items)
      m_array[pos++] = item;
  }


  njson get(const std::size_t pos) const
  {
    return m_array[pos];
  }


  njson getRange(const std::size_t start) const
  {
    return getRange(start, m_size);
  }


  njson getRange(const std::size_t start, std::size_t stop) const
  {
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


  void swap(const std::size_t posA, const std::size_t posB)
  {
    std::iter_swap( std::next(m_array.begin(), posA),
                    std::next(m_array.begin(), posB));
  }


  void clear(const std::size_t start)
  {
    clear(start, m_size);
  }


  void clear(const std::size_t start, const std::size_t stop)
  {
    PLOGD << "Array::clear(): " << start << " to " << stop;

    const auto itStart = std::next(m_array.begin(), start);
    const auto itEnd = std::next(m_array.begin(), std::min<std::size_t>(m_size, stop));

    for (auto it = itStart ; it != itEnd; ++it)
      *it = T{};
  }


  std::size_t size() const noexcept
  {
    return m_size;
  }

private:
  std::vector<T> m_array;
  std::size_t m_size;
};


using OArray = Array<njson>;
using IArray = Array<std::int64_t>;
using SArray = Array<std::string>;

}
}

#endif
