#ifndef _NDB_CACHEMAP_
#define _NDB_CACHEMAP_

#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>


namespace nemesis { namespace core {


class CacheMap
{
  using Map = ankerl::unordered_dense::segmented_map<cachedkey, cachedvalue2>;
  using CacheMapIterator = Map::iterator;
  using CacheMapConstIterator = Map::const_iterator;

public:

  auto set (const cachedkey& key, cachedvalue2&& value) -> std::pair<CacheMapIterator, bool>
  {
    return m_map.insert_or_assign(key, std::move(value));
  }


  auto get (const cachedkey& key) const -> std::tuple<bool, cachedvalue2>
  {
    if (const auto it = m_map.find(key) ; it != m_map.cend())
      return {true, it->second};
    else
      return {false, njson::null()};
  };


  bool add (const cachedkey& key, cachedvalue2&& value)
  {
    const auto [ignore, added] = m_map.try_emplace(key, std::move(value));
    return added;
  }


  bool remove (const cachedkey& key)
  {
    return m_map.erase(key) != 0U;
  };

  
  auto clear() -> std::tuple<bool, std::size_t>
  {
    auto size = m_map.size();
    bool valid = true;

    try
    {
      m_map.clear();
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

  
  /*
  auto append (const cachedkey& key, njson&& value)
  {
    RequestStatus status = RequestStatus::Ok;

    if (const auto it = m_map.find(key) ; it != m_map.cend())
    {
      if (it->second.type() == njson::value_t::string && value.is_string())
      {
        if (value.is_string())
          it->second.get_ref<njson::string_t&>().append(std::move(value));
        else
          status = RequestStatus::ValueTypeInvalid;
      }
      else if (it->second.type() == njson::value_t::array)
      {
        if (value.is_array())
        {
          for (auto&& item : value)
            it->second.insert(it->second.end(), std::move(item));
        }
        else
          it->second.insert(it->second.end(), std::move(value));        
      }
      else if(it->second.type() == njson::value_t::object && value.is_object())
        it->second.update(value.begin(), value.end(), true);
      else
        status = RequestStatus::ValueTypeInvalid;
    }
    else 
      status = RequestStatus::KeyNotExist;

    return status;
  }
  */


  bool contains (const cachedkey& key)
  {
    return m_map.contains(key);
  };

  
  // TODO review this, result as json may not be required: it's always returning strings (path or keys)
  auto find (const njson& contents, const bool findPaths, njson& result) const
  {
    namespace jsonpath = jsoncons::jsonpath;
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


  std::tuple<bool, std::size_t> update (const cachedkey& key, const std::string& path, njson&& value) 
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
