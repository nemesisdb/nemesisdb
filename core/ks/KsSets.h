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


  RequestStatus addKey (const ksname& name, const cachedkey& key)
  {
    if (auto it = m_sets.find(name); it != m_sets.cend())
    {
      auto [ignore, inserted] = it->second.emplace(key);
      return inserted ? RequestStatus::Ok : RequestStatus::KeyExists;
    }
    
    return RequestStatus::KeySetNotExist;
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
  ankerl::unordered_dense::map<ksname, ksset> m_sets;
  mutable std::mutex m_mux;
};

} // ks 
} // core
} // fusion


#endif
