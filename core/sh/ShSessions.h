#ifndef NDB_CORE_SHSESSIONS_H
#define NDB_CORE_SHSESSIONS_H

#include <string>
#include <string_view>
#include <chrono>
#include <map>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/CacheMap.h>
#include <core/kv/KvCommon.h>


namespace nemesis { namespace sh {

using namespace nemesis::core;


class Sessions
{

public:
  struct ExpireInfo
  {
    SessionExpireTime time{};
    SessionDuration duration{};
    bool deleteOnExpire{false};
    bool extendOnSetAdd{false};
    bool extendOnGet{false};
  };

  struct Session
  {
    CacheMap map;
    ExpireInfo expireInfo;
    SessionToken token;    

    bool shared{false};
    bool expires{false}; 
  };

  using SessionsMap = ankerl::unordered_dense::segmented_map<SessionToken, Session>;


private:
  struct ExpiryTracking
  {
    ExpireInfo expiry;
    SessionToken token;
  };


public:

  std::optional<std::reference_wrapper<CacheMap>> start (const SessionToken& token, const bool shared, ExpireInfo expiry)
  {
    if (m_sessions.contains(token))
      return {};  // TODO check this, if shared:true and a session with this name already exists
    else
    {
      if (expiry.duration == SessionDuration::zero())  // never expires
        m_sessions.emplace(token, Session{.token = token, .shared = shared, .expires = false});
      else
      {
        const auto expireTime = SessionClock::now() + expiry.duration;
        
        expiry.time = expireTime;

        m_sessions.emplace(token, Session{.expireInfo = expiry, .token = token, .shared = shared, .expires = true});
        m_expiry.emplace(expireTime, ExpiryTracking{.expiry = expiry, .token = token});
      }

      return m_sessions.at(token).map;
    }
  }


  bool contains (const SessionToken& token) const
  {
    return m_sessions.contains(token);
  }
  

  std::tuple<bool, bool> openShared (const SessionToken& token) const
  {
    if (const auto sesh = m_sessions.find(token); sesh != m_sessions.end())
      return {true, sesh->second.shared};

    return {false, false};
  }


  bool end (const SessionToken& token)
  {
    bool ended = false;
    if (const auto sesh = m_sessions.find(token); sesh != m_sessions.end())
    {
      if (sesh->second.expires)
      {
        // m_expiry (multimap) uses expiry time as key so there may be several session with same expiry time
        if (const auto it = m_expiry.equal_range(sesh->second.expireInfo.time); it.first != m_expiry.end())
        {
          const auto itSeshEntry = std::find_if(it.first, it.second, [&token](const auto pair) {return pair.second.token == token;});
          if (itSeshEntry != it.second)
          {
            m_expiry.erase(itSeshEntry);
            ended = true;
          }
        }

        m_sessions.erase(sesh);
      }
      else
      {        
        m_sessions.erase(sesh);
        ended = true;
      }
    }
    
    return ended;
  }

  
  std::size_t clear ()
  {
    // TODO m_sessions.clear() or m_sessions = SessionsMap{}  don't actually deallocate the bucket memory
    //      the advantage is the memory is available for subsequence set commands, but also means memory is not released and
    //      this may be preferred. Investigate.
    const auto count = m_sessions.size();
    m_sessions.clear(); 
    m_expiry.clear();
    return count;
  }


  SessionsMap::size_type countSessions() const
  {
    return m_sessions.size();
  }


  std::size_t countKeys() const
  {
    std::size_t count = 0 ;
    std::for_each(m_sessions.cbegin(), m_sessions.cend(), [&count](const auto& pair)
    {
      count += pair.second.map.count();
    });
    return count;
  }


  std::optional<std::reference_wrapper<CacheMap>> getMap (const SessionToken& token)
  {
    if (auto it = m_sessions.find(token) ; it == m_sessions.end())
      return {};
    else
      return {it->second.map};
  }


  const SessionsMap& getSessions () const
  {
    return m_sessions;
  }


  std::optional<std::reference_wrapper<Session>> get (const SessionToken& token)
  {
    if (const auto it = m_sessions.find(token) ; it == m_sessions.end())
      return {};
    else
      return {it->second};
  }


  void handleExpired ()
  {    
    using Iterator = std::multimap<SessionExpireTime, ExpiryTracking>::iterator;

    if (m_expiry.empty())
      return;

    const auto now = SessionClock::now();    
    const auto itLimit = m_expiry.lower_bound(now);
        
    std::vector<Iterator> expired;
    for (Iterator it = m_expiry.begin(); it != itLimit ; ++it)
      expired.emplace_back(it);

    PLOGD << "expired: " << expired.size();

    for (const auto& itExpired : expired)
    {
      if (itExpired->second.expiry.deleteOnExpire)
      {
        m_sessions.erase(itExpired->second.token);
        m_expiry.erase(itExpired);
      }
      else
      {
        auto expiredNode = m_expiry.extract(itExpired);      
        const SessionExpireTime expireTime = now + expiredNode.mapped().expiry.duration;
        
        expiredNode.key() = expireTime;
        
        auto& session = m_sessions.at(expiredNode.mapped().token);
        session.map.clear();
        session.expireInfo.time = expireTime;
        
        m_expiry.insert(std::move(expiredNode));
      }
    }
  }


  void updateExpiry (Session& sesh)
  {
    if (auto itExpire = m_expiry.find(sesh.expireInfo.time); itExpire != m_expiry.end())
    {
      const auto expireEntry = std::find_if(itExpire, m_expiry.end(), [&sesh](auto expiry) { return expiry.second.token == sesh.token; });
      if (expireEntry != m_expiry.end())
      {
        // m_expiry uses the expire time as a key so we need to extract, update time and insert with new time
        auto node = m_expiry.extract(expireEntry);

        const auto expireTime = SessionClock::now() + node.mapped().expiry.duration;

        sesh.expireInfo.time = expireTime;
        
        node.mapped().expiry.time = expireTime;
        node.key() = expireTime;

        m_expiry.insert(std::move(node));
      }
    }
  }


private:
  SessionsMap m_sessions;
  std::multimap<SessionExpireTime, ExpiryTracking> m_expiry;
  // using priority_queue introduces complexities for updateExpiry() because it is not simple to update the priority_queue from the Session.
  // more work needed, put aside for now 
};


}
}

#endif
