#ifndef _NDB_CACHEMAP_
#define _NDB_CACHEMAP_

#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core {


class CacheMap
{
  using Map = ankerl::unordered_dense::segmented_map<cachedkey, cachedvalue>;
  using CacheMapIterator = Map::iterator;
  using CacheMapConstIterator = Map::const_iterator;

public:
  
  void set (cachedkey key, cachedvalue value)
  {
    m_map.insert_or_assign(std::move(key), std::move(value));
  }


  std::optional<std::reference_wrapper<const cachedvalue>> get (const cachedkey& key) const
  {
    if (const auto it = m_map.find(key) ; it != m_map.cend())
      return {it->second};
    else
      return {};
  };


  void add (const cachedkey& key, cachedvalue&& value)
  {
    m_map.try_emplace(key, std::move(value));
  }


  bool remove (const cachedkey& key)
  {
    return m_map.erase(key) != 0U;
  };

  
  std::tuple<bool, std::size_t> clear()
  {
    auto size = m_map.size();
    bool valid = true;

    try
    {
      m_map.clear();
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

  
  bool contains (const cachedkey& key)
  {
    return m_map.contains(key);
  };

  
  // TODO review this, result as json may not be required: it's always returning strings (path or keys)
  auto find (const njson& contents, const bool findPaths, njson& result) const
  {
    const auto& path = contents.at("path").as_string();
    
    try
    {
      if (contents.at("keys").empty())
      {
        for(auto& kv : m_map)
        {       
          if (auto queryResult = jsonpath::json_query(kv.second, path, jsonpath::result_options::path | jsonpath::result_options::nodups); !queryResult.empty())
          {
            if (findPaths)
              for(auto& i : queryResult.array_range())
                result.emplace_back(std::move(i.as_string()));
            else
              result.emplace_back(kv.first);
          }
        }
      }
      else
      {
        for (auto& kv : contents.at("keys").array_range())
        {
          const auto& key = kv.as_string();

          if (auto entry = m_map.find(key); entry != m_map.cend())
          {
            if (auto queryResult = jsonpath::json_query(entry->second, path, jsonpath::result_options::path | jsonpath::result_options::nodups); !queryResult.empty())
            {
              if (findPaths)
                for(auto& i : queryResult.array_range())
                  result.emplace_back(std::move(i.as_string()));
              else
                result.emplace_back(key);
            } 
          }
        }
      }
    }
    catch (.../*const jsonpath::jsonpath_error&*/)
    {
      return false;
    }

    return true;
  }


  std::tuple<bool, std::size_t> update (const cachedkey& key, const std::string& path, cachedvalue&& value) 
  {
    namespace jsonpath = jsoncons::jsonpath;

    if (auto entry = m_map.find(key); entry != m_map.cend())
    {
      std::size_t updated = 0;

      jsonpath::json_replace(entry->second, path, [&updated, value = std::move(value)](const std::string& /*path*/, njson& currentValue)
      {
        currentValue = std::move(value);
        ++updated;
      });

      return {true, updated};
    }

    return {false, 0U};
  }


  njson keys()
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
  Map m_map;
};

} // ns core
} // ns nemesis

#endif
