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
does provide isInbounds() for users.
*/
class Array
{
public:
  
  Array(const std::size_t size) : m_size(size)
  {
    m_array.resize(m_size); // initialised as empty json objects: {}
  }


  static bool isRequestedSizeValid(const std::size_t size)
  {
    return size == std::clamp<std::size_t>(size, 1, 1'000); // TODO quite arbitrary
  }


  bool isInBounds(const std::size_t pos) const noexcept
  {
    return pos < m_size;
  }


  bool isSetInBounds(const std::size_t start, const std::size_t setSize)
  {
    return start < m_size && start + setSize <= m_size;
  }


  bool isGetInBounds(const std::size_t start, const std::size_t stop)
  {
    return start < m_size && start+(stop-start) <= m_size;
  }


  void set(const std::size_t pos, const njson& item)
  {
    m_array[pos] = item;
  }


  void setRange(std::size_t pos, const njson& items)
  {    
    for (const auto& item : items.array_range())
      m_array[pos++] = item;
  }


  njson get(const std::size_t pos) const
  {
    return m_array[pos];
  }


  njson getRange(const std::size_t start, const std::size_t stop) const
  {
    const auto itStart = std::next(m_array.cbegin(), start);
    const auto itEnd = std::next(itStart, std::min<std::size_t>(m_size, stop-start));
    const auto rangeSize = std::distance(itStart, itEnd);

    njson rsp = njson::make_array(std::clamp<std::size_t>(rangeSize, rangeSize, 100));

    PLOGD << "getRange(): rsp size " << rsp.size();
    
    std::size_t i = 0;
    std::for_each_n(itStart, rangeSize, [&rsp, &i](const auto& json)
    {
      rsp[i++] = json;
    });

    return rsp;
  }


  void swap(const std::size_t posA, const std::size_t posB)
  {
    std::iter_swap( std::next(m_array.begin(), posA),
                    std::next(m_array.begin(), posB));
  }



  std::size_t size() const noexcept
  {
    return m_size;
  }

private:
  std::vector<njson> m_array;
  std::size_t m_size;
};

}
}

#endif
