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
  
  Array(const std::size_t sizeHint = 10) : m_size(std::clamp<std::size_t>(sizeHint, 1, 1'000))
  {
    m_array.resize(m_size); // initialised as empty json object: {}
  }


  bool isInbounds(const std::size_t pos) const noexcept
  {
    return pos <= m_size-1;
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
    const auto rangeSize = stop-start+1;

    njson rsp = njson::make_array(std::clamp<std::size_t>(rangeSize, rangeSize, 100));

    PLOGD << "getRange(): rsp size " << rsp.size();
    
    std::size_t i = 0;
    std::for_each_n(std::next(m_array.cbegin(), start), rangeSize, [&rsp, &i](const auto& json)
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
