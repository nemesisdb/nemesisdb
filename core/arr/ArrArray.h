#ifndef NDB_CORE_ARRARRAY_H
#define NDB_CORE_ARRARRAY_H

#include <functional>
#include <string_view>
#include <core/NemesisCommon.h>
#include <core/arr/ArrCommands.h>
#include <core/arr/ArrArray.h>


namespace nemesis { namespace arr {


using std::vector;


/*
A fixed-sized std::vector.
*/
class Array
{
  

public:
  
  Array(const std::size_t sizeHint = 10) : m_size(std::clamp<std::size_t>(sizeHint, 10, 1'000))
  {
    m_array.resize(m_size);
  }

  bool isInbounds(const std::size_t pos)
  {
    return pos <= m_size-1;
  }

  void set(const std::size_t pos, njson& item)
  {
    m_array[pos] = item;
    PLOGD << m_array[pos].to_string();
  }

  njson get(const std::size_t pos)
  {
    return m_array[pos];
  }

  njson getRange(const std::size_t start, const std::size_t stop)
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

private:
  std::vector<njson> m_array;
  std::size_t m_size;
};

}
}

#endif
