#ifndef _FC_CACHEMAP_
#define _FC_CACHEMAP_

#include <regex>
#include <ankerl/unordered_dense.h>
#include <core/FusionCommon.h>


namespace fusion { namespace core {


class CacheMap
{
  using Map = ankerl::unordered_dense::segmented_map<cachedkey, cachedvalue>;
  using CacheMapIterator = Map::iterator;
  using CacheMapConstIterator = Map::const_iterator;

public:

  auto set (fcjson& contents) -> std::pair<Map::iterator, bool>
  {
    const auto& key = contents.begin().key();
    auto& value = contents.begin().value(); 
    return m_map.insert_or_assign(key, std::move(value));
  }

  auto set (const cachedkey& key, cachedvalue&& value) -> std::pair<CacheMapIterator, bool>
  {
    return m_map.insert_or_assign(key, std::move(value));
  }


  void setQ (fcjson& contents)
  {
    const auto& key = contents.begin().key(); 
    auto& value = contents.begin().value(); 
    m_map.insert_or_assign(key, std::move(value));
  }


  auto get (fcjson& contents) const -> std::pair<bool, cachedpair>
  {
    if (const auto it = m_map.find(contents) ; it != m_map.cend())
      return std::make_pair(true, cachedpair {{it->first, it->second}});
    else
      return std::make_pair(false, cachedpair{});
  };


  auto add (fcjson& contents) -> std::tuple<bool, std::string>
  {
    const auto& key = contents.items().begin().key();
    auto& value = contents.items().begin().value(); 

    const auto [ignore, added] = m_map.emplace(key, std::move(value));
    return std::make_tuple(added, key);
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


  std::size_t count()
  {
    return m_map.size();
  }

  
  auto append (fcjson& contents)
  {
    RequestStatus status = RequestStatus::Ok;

    const auto& key = contents.begin().key();
    if (const auto it = m_map.find(key) ; it != m_map.cend())
    {
      switch (contents.begin().value().type())
      {
        case fcjson::value_t::array:
        {
          for (auto& item : contents.at(key))
            it->second.insert(it->second.end(), std::move(item));
        }
        break;

        case fcjson::value_t::object:
          it->second.insert(contents.begin().value().begin(), contents.begin().value().end());
        break;

        case fcjson::value_t::string:
          it->second.get_ref<fcjson::string_t&>().append(contents.begin().value());
        break;

        default:
          status = RequestStatus::ValueTypeInvalid;
        break;
      }
    }
    else
      status = RequestStatus::KeyNotExist;
    

    return status;
  };


  bool contains (const fcjson& contents)
  {
    return m_map.contains(contents);
  };

      
  RequestStatus arrayMove (const fcjson& contents)
  {
    auto& key = contents.begin().key();
    auto& positions = contents.begin().value();
    
    if (positions.size() != 2U && positions.size() != 1U)
      return RequestStatus::ValueSize;
    else
    {
      if (auto it = m_map.find(key) ; it == m_map.cend())
        return RequestStatus::KeyNotExist;
      else  [[likely]]
      {
        if (auto& array = it->second; !array.is_array())
          return RequestStatus::ValueTypeInvalid; 
        else if (array.empty())
          return RequestStatus::OutOfBounds; 
        else
        {
          auto isIndexValidType = [](const json& array, const std::size_t index)
          {
            return array[index].is_number_unsigned();
          };

          if (positions.size() == 1U && !isIndexValidType(positions, 0U))
            return RequestStatus::ValueTypeInvalid;
          else if (positions.size() == 2U && (!isIndexValidType(positions, 0U) || !isIndexValidType(positions, 1U)))
            return RequestStatus::ValueTypeInvalid;
          else
          {
            const std::int64_t currPos = positions[0U];
            const std::int64_t newPos = positions.size() == 1U ? array.size() : positions[1U].get<std::int64_t>(); // at the end if no position supplied

            if (currPos < 0 || currPos > array.size() - 1 || newPos < 0)
              return RequestStatus::OutOfBounds;
            else if (currPos == newPos)
              return RequestStatus::Ok;
            else
            {
              array.insert(std::next(array.cbegin(), newPos), std::move(array[currPos]));
              array.erase(currPos > newPos ? currPos+1 : currPos);

              return RequestStatus::Ok;
            }
          }
        }
      }
    }
  };


  auto find (const fcjson& contents, const KvFind& find)
  {
    // this is sent to all workers. Each runs the "path" search on all keys, then applies the condition on each result.
    // the keys which pass the condition, are returned
    
    auto& [opString, handler] = findConditions.getOperation(find.condition);

    const auto haveRegex = !contents.at("keyrgx").get_ref<const std::string&>().empty();
    fcjson::json_pointer path {contents.at("path")};
    fcjson::const_reference value = contents.at(opString);
    
    std::vector<cachedkey> keys;
    keys.reserve(100U);  // TODO

    auto valueMatch = [&handler, &keys, &value, &path](std::pair<cachedkey, cachedvalue>& kv)
    {
      if (path.empty() && handler(kv.second, value))
        return true;
      else if (kv.second.contains(path) && handler(kv.second.at(path), value))
        return true;
      else
        return false;
    };

    const std::regex keyRegex{contents.at("keyrgx").get_ref<const std::string&>()};
    
    for(auto& kv : m_map)
    {
      if (haveRegex)
      {     
        if (std::regex_match(kv.first, keyRegex) && valueMatch(kv))
          keys.emplace_back(kv.first);
      }
      else if (valueMatch(kv))
        keys.emplace_back(kv.first);
    }

    keys.shrink_to_fit();

    return std::move(keys);
  };


private:

  auto doAdd (fcjson& contents) -> std::tuple<bool, std::string>
  {
    const auto& key = contents.items().begin().key();
    auto& value = contents.items().begin().value(); 

    const auto [ignore, added] = m_map.emplace(key, std::move(value));
    return std::make_tuple(added, key);
  };

private:
  Map m_map;
};

} // ns core
} // ns fusion

#endif
