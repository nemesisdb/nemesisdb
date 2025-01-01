#ifndef _NDB_CACHEMAP2_
#define _NDB_CACHEMAP2_

// This should be in kv dir

#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>


namespace nemesis { namespace kv {


class CacheMap
{
  using Map = ankerl::unordered_dense::segmented_map<cachedkey, CachedValue>;
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


  template<flexbuffers::Type FlexT, typename ValueT>
  void set (const std::string& key, const ValueT& v)
  {
    ValueExtractF extract{nullptr};

    if constexpr (FlexT == flexbuffers::Type::FBT_INT)
      extract = extractInt;
    else if constexpr (FlexT == flexbuffers::Type::FBT_UINT)
      extract = extractUInt;
    else if constexpr (FlexT == flexbuffers::Type::FBT_FLOAT)
      extract = extractFloat;
    else if constexpr (FlexT == flexbuffers::Type::FBT_BOOL)
      extract = extractBool;
    else if constexpr (FlexT == flexbuffers::Type::FBT_STRING)
      extract = extractString;
    else
      static_assert("Unsupported FlexBuffer::Type");

    CachedValue cv {.value = v, .extract = extract};
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
          const auto& cachedValue = it->second;
          
          cachedValue.extract(pKey, cachedValue.value, fb);
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
    // njson keys = njson::make_array();
    
    // for(const auto& it : m_map)
    //   keys.emplace_back(it.first);

    // return keys;
    return njson{};
  }


  const Map& map () const
  {
    return m_map;
  }


private:

  static void extractInt(const char * key, const ValueVariant& value, FlexBuilder& fb)
  {
    fb.Int(key, std::get<CachedValue::GET_INT>(value));
  }

  static void extractUInt(const char * key, const ValueVariant& value, FlexBuilder& fb)
  {
    fb.UInt(key, std::get<CachedValue::GET_UINT>(value));
  }

  static void extractFloat(const char * key, const ValueVariant& value, FlexBuilder& fb)
  {
    fb.Float(key, std::get<CachedValue::GET_DBL>(value));
  }

  static void extractBool(const char * key, const ValueVariant& value, FlexBuilder& fb)
  {
    fb.Bool(key, std::get<CachedValue::GET_BOOL>(value));
  }

  static void extractString(const char * key, const ValueVariant& value, FlexBuilder& fb)
  {
    fb.String(key, std::get<CachedValue::GET_STR>(value));
  }


private:
  Map m_map;
};

} // ns kv
} // ns nemesis

#endif
