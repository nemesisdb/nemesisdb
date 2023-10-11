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

    KeySetIt it = m_sets.find(name);
    
    if (it == m_sets.end())
      it = m_sets.try_emplace(name, ksset{}).first;
    
    for (auto& key : keyArray.items())
    {
      if (!isKeyValid(key.value()))
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
