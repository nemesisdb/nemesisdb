#ifndef FC_CORE_KSSETS_H
#define FC_CORE_KSSETS_H


#include <vector>
#include <mutex>
#include <ankerl/unordered_dense.h>
#include <core/ks/KsCommon.h>
#include <core/kv/KvCommon.h>


namespace fusion { namespace core { namespace ks {

using namespace fusion::core::kv;

// Used to group keys into a single set. 
// A KeySet can be used in commands to restrict which keys are affected.
class Sets
{
  using KeySet = ankerl::unordered_dense::map<ksname, ksset>;
  using KeySetIt = ankerl::unordered_dense::map<ksname, ksset>::iterator;
  using KeySetConstIt = ankerl::unordered_dense::map<ksname, ksset>::const_iterator;


public:

  Sets() = default;
  
  Sets(const Sets&) = delete;
  Sets& operator=(const Sets&) = delete;


  bool create (const ksname& name)
  {
    std::scoped_lock lck{m_mux};

    auto [ignore, added] = m_sets.try_emplace(name, ksset{});
    return added;
  }


  RequestStatus addKeys (const ksname& name, const fcjson& keyArray)
  {
    std::scoped_lock lck{m_mux};

    RequestStatus status = RequestStatus::Ok;

    auto it = m_sets.find(name);
    
    if (it == m_sets.end())
      it = m_sets.try_emplace(name, ksset{}).first;

    for (auto& key : keyArray.items())
    {
      if ((!key.value().is_string()) || !isKeyValid(key.value()))
        status = RequestStatus::KeyAddFailed;
      else
      {
        try
        {
          it->second.emplace(key.value());
        }
        catch (...)
        {
          status = RequestStatus::KeyAddFailed;
        }
      }
    }
    
    return status;
  }


  fc_always_inline bool getSet (const ksname& name, fcjson& set)
  {
    std::scoped_lock lck{m_mux};

    if (auto it = m_sets.find(name) ; it != m_sets.cend())
    {
      // json lib uses vector as array container, so can pre-size using this constructor
      set = fcjson(it->second.size(), "");
      auto itKey = it->second.cbegin();

      for (auto& s : set.items())
      {
        s.value() = *itKey;
        ++itKey;
      }

      return true;
    }

    return false;
  }
  

  fc_always_inline RequestStatus removeKeys (const ksname& name, const fcjson& keyArray)
  {
    std::scoped_lock lck{m_mux};

    RequestStatus status = RequestStatus::Ok;

    if (auto itSet = m_sets.find(name); itSet != m_sets.end())
    {
      for (auto& key : keyArray.items())
      {
        if (!key.value().is_string())
          status = RequestStatus::KeyRemoveFailed;
        else
          itSet->second.erase(key.value());
      }

      return status;
    }

    return RequestStatus::KeySetNotExist;
  }


  fc_always_inline RequestStatus clearSet (const ksname& name)
  {
    std::scoped_lock lck{m_mux};

    RequestStatus status = RequestStatus::Ok;
    
    if (auto itSet = m_sets.find(name); itSet != m_sets.end())
    {
      try
      {
        itSet->second.clear();
      }
      catch (...)
      {
        status = RequestStatus::KeySetRemoveAllFailed;
      }
    }

    return status;
  }


  fc_always_inline RequestStatus deleteSet (const ksname& name)
  {
    std::scoped_lock lck{m_mux};

    RequestStatus status = RequestStatus::Ok;

    try
    {
      status = m_sets.erase(name) ? RequestStatus::Ok : RequestStatus::KeySetNotExist;
    }
    catch (...)
    {
      status = RequestStatus::KeySetRemoveAllFailed;
    }

    return status;
  }


  fc_always_inline RequestStatus clear ()
  {
    std::scoped_lock lck{m_mux};

    RequestStatus status = RequestStatus::Ok;

    try
    {
      m_sets.clear();
    }
    catch (...)
    {
      status = RequestStatus::KeySetClearFailed;
    }

    return status;
  }


  fc_always_inline RequestStatus move (const ksname& source, const ksname& target, const cachedkey& key)  
  {
    // can't extract() from ankerl set because non-standard API empties source (rather than a single node)

    std::scoped_lock lck{m_mux};

    RequestStatus status = RequestStatus::Ok;

    auto itSource = m_sets.find(source);
    auto itTarget = m_sets.find(target);

    if (itSource == m_sets.end() || itTarget == m_sets.end())
      status = RequestStatus::KeySetNotExist;
    else
    {
      if (!itSource->second.contains(key))
        status = RequestStatus::KeyNotExist;
      else
      {
        try
        {        
          itTarget->second.emplace(key);
        }
        catch (...)
        {
          // no harm done, nothing to recover from
          status = RequestStatus::KeyMoveFailed;
        }

        try
        {
          itSource->second.erase(key);
        }
        catch (...)
        {
          status = RequestStatus::KeyMoveFailed;
          // undo adding key to target
          itTarget->second.erase(key);
        }
      } 
    }

    return status;
  }

  
  fcjson list() const
  {
    std::scoped_lock lck{m_mux};
    
    fcjson list = fcjson::array();
    
    std::for_each(m_sets.cbegin(), m_sets.cend(), [&list](const auto& pair) { list.emplace_back(pair.first); });

    return list;
  }


  bool contains(const ksname& name) const
  {
    std::scoped_lock lck{m_mux};
    return m_sets.contains(name);
  }


  bool contains(const ksname& name, const cachedkey& key) const
  {
    std::scoped_lock lck{m_mux};
    if (auto it = m_sets.find(name); it != m_sets.cend())
      return it->second.contains(key);
    
    return false;
  }
  

private:
  KeySet m_sets;
  mutable std::mutex m_mux;
};

} // ks 
} // core
} // fusion


#endif
