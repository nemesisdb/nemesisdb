#ifndef _NDB_CACHEMAP2_
#define _NDB_CACHEMAP2_

// This should be in kv dir

#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>


namespace nemesis { 


class CacheMap2
{
  using Map = ankerl::unordered_dense::segmented_map<cachedkey, kv::cachedvalue2>;
  using CacheMapIterator = Map::iterator;
  using CacheMapConstIterator = Map::const_iterator;

public:
  
  CacheMap2& operator=(CacheMap2&&) = default; // required by Map::erase()
  CacheMap2(CacheMap2&&) = default;

  CacheMap2& operator=(const CacheMap2&) = delete;   
  CacheMap2(CacheMap2&) = delete;

  
  CacheMap2 (const std::size_t buckets = 0) : m_map(buckets) 
  {
  }


  /*
  void set(const std::string& key, const int64_t v)
  {
    kv::cachedvalue2 cv { .type = flexbuffers::Type::FBT_INT,
                          .value = v};

    m_map.insert_or_assign(key, cv);
  }

  void set(const std::string& key, const uint64_t v)
  {
    kv::cachedvalue2 cv { .type = flexbuffers::Type::FBT_UINT,
                          .value = v};

    m_map.insert_or_assign(key, cv);
  }


  void set(const std::string& key, const bool v)
  {
    kv::cachedvalue2 cv { .type = flexbuffers::Type::FBT_BOOL,
                          .value = v};

    m_map.insert_or_assign(key, cv);
  }


  void set(const std::string& key, const std::string& v)
  {
    kv::cachedvalue2 cv { .type = flexbuffers::Type::FBT_STRING,
                          .value = v};

    m_map.insert_or_assign(key, cv);
  }


  void set(const std::string& key, const double v)
  {
    kv::cachedvalue2 cv { .type = flexbuffers::Type::FBT_FLOAT,
                          .value = v};

    m_map.insert_or_assign(key, cv);
  }
  */

  template<flexbuffers::Type FlexT, typename ValueT>
  void set (const std::string& key, const ValueT& v)
  {
    kv::cachedvalue2 cv { .type = FlexT,
                          .value = v};

    m_map.insert_or_assign(key, cv);
  }


  std::optional<std::reference_wrapper<const kv::cachedvalue2>> get (const cachedkey& key) const
  {
    if (const auto it = m_map.find(key) ; it != m_map.cend())
      return {it->second};

    return {};
  };


  void add (cachedkey key, kv::cachedvalue2 value)
  {
    //m_map.try_emplace(std::move(key), std::move(value));
  }


  void remove (const cachedkey& key)
  {
    //m_map.erase(key);
  };

  
  std::tuple<bool, std::size_t> clear()
  {
    // auto size = m_map.size();
    // bool valid = true;

    // try
    // {
    //   m_map.replace(Map::value_container_type{});
    // }
    // catch (...)
    // {
    //   valid = false;
    //   size = 0U;
    // }
    
    // return std::make_pair(valid, size);

    return {false, 0};
  };


  std::size_t count() const
  {
    return m_map.size();
  }

  
  bool contains (const cachedkey& key) const
  {
    return m_map.contains(key);
  };


  njson keys() const
  {
    njson keys = njson::make_array();
    
    for(const auto& it : m_map)
      keys.emplace_back(it.first);

    return keys;
  }


  const Map& map () const
  {
    return m_map;
  }


private:

  

private:
  Map m_map;
};

} // ns nemesis

#endif
