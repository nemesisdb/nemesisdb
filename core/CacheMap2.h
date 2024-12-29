#ifndef _NDB_CACHEMAP2_
#define _NDB_CACHEMAP2_

#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>


namespace nemesis { 


class CacheMap
{
  using Map = ankerl::unordered_dense::segmented_map<cachedkey, cachedvalue2>;
  using CacheMapIterator = Map::iterator;
  using CacheMapConstIterator = Map::const_iterator;

public:
  
  CacheMap& operator=(CacheMap&&) = default; // required by Map::erase()
  CacheMap(CacheMap&&) = default;

  CacheMap& operator=(const CacheMap&) = delete;   
  CacheMap(CacheMap&) = delete;

  
  CacheMap (const std::size_t buckets = 0) : m_map(buckets) 
  {
  }


  void set (cachedkey key, cachedvalue2 value)
  {
    m_map.insert_or_assign(std::move(key), std::move(value));
  }


  std::optional<std::reference_wrapper<const cachedvalue2>> get (const cachedkey& key) const
  {
    if (const auto it = m_map.find(key) ; it != m_map.cend())
      return {it->second};

    return {};
  };


  void add (cachedkey key, cachedvalue2 value)
  {
    m_map.try_emplace(std::move(key), std::move(value));
  }


  void remove (const cachedkey& key)
  {
    m_map.erase(key);
  };

  
  std::tuple<bool, std::size_t> clear()
  {
    auto size = m_map.size();
    bool valid = true;

    try
    {
      m_map.replace(Map::value_container_type{});
    }
    catch (...)
    {
      valid = false;
      size = 0U;
    }
    
    return std::make_pair(valid, size);
  };


  std::size_t count() const
  {
    return m_map.size();
  }

  
  bool contains (const cachedkey& key) const
  {
    return m_map.contains(key);
  };


  std::vector<cachedkey> keys() const
  {
    std::vector<cachedkey> keys;
    keys.reserve(m_map.size());
    
    for(const auto& it : m_map)
      keys.emplace_back(it.first);

    return keys;
  }


  const Map& map () const
  {
    return m_map;
  }

private:
  Map m_map;
};

} // ns nemesis

#endif
