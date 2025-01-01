#ifndef _NDB_CACHEMAP2_
#define _NDB_CACHEMAP2_

// This should be in kv dir

#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>


namespace nemesis { namespace kv {


class CacheMap
{
  using ScalarMap = ankerl::unordered_dense::segmented_map<cachedkey, CachedValue>;
  //using StringMap = ankerl::unordered_dense::segmented_map<cachedkey, kv::CachedString>;
  using CacheMapIterator = ScalarMap::iterator;
  using CacheMapConstIterator = ScalarMap::const_iterator;

public:
  
  CacheMap& operator=(CacheMap&&) = default; // required by ScalarMap::erase()
  CacheMap(CacheMap&&) = default;

  CacheMap& operator=(const CacheMap&) = delete;   
  CacheMap(CacheMap&) = delete;

  
  CacheMap (const std::size_t buckets = 0) : m_map(buckets) 
  {
  }


  template<flexbuffers::Type FlexT, typename ValueT>
  void set (const std::string& key, const ValueT& v)
  {
    CachedValue cv {.type = FlexT, .value = v};
    m_map.insert_or_assign(key, cv);
  }


  void get (const KeyVector& keys, FlexBuilder& fb)
  {
    fb.Map([&]()
    {
      for (const auto& key : keys)
      { 
        if (const auto& it = m_map.find(key->str()); it != m_map.cend())
        {
          const auto pKey = key->c_str();
          const auto& cv = it->second;
          
          switch (cv.type)
          {
            using enum flexbuffers::Type;

            case FBT_INT:
              fb.Int(pKey, std::get<CachedValue::GET_INT>(cv.value));
            break;

            case FBT_UINT:
              fb.UInt(pKey, std::get<CachedValue::GET_UINT>(cv.value));
            break;

            case FBT_FLOAT:
              fb.Float(pKey, std::get<CachedValue::GET_DBL>(cv.value));
            break;

            case FBT_BOOL:
              fb.Bool(pKey, std::get<CachedValue::GET_BOOL>(cv.value));
            break;

            case FBT_STRING:
              fb.String(pKey, std::get<CachedValue::GET_STR>(cv.value));
            break;

            default:
            break;
          }
        }
      }      
    });
  }

  
  void add (cachedkey key, CachedValue value)
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
    //   m_map.replace(ScalarMap::value_container_type{});
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
    // njson keys = njson::make_array();
    
    // for(const auto& it : m_map)
    //   keys.emplace_back(it.first);

    // return keys;
    return njson{};
  }


  const ScalarMap& map () const
  {
    return m_map;
  }


private:

  

private:
  ScalarMap m_map;
  //StringMap m_stringMap;
};

} // ns kv
} // ns nemesis

#endif
