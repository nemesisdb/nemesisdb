#ifndef _FC_SHSESSIONS_
#define _FC_SHSESSIONS_

#include <string>
#include <string_view>
#include <chrono>
#include <map>
#include <ankerl/unordered_dense.h>
#include <core/NemesisCommon.h>
#include <core/CacheMap.h>
#include <core/kv/KvCommon.h>


namespace nemesis { namespace core { namespace kv {

using namespace nemesis::core;


class Sessions
{

private:
  struct ExpireInfo
  {
    SessionExpireTime time;
    SessionDuration duration;
    bool deleteOnExpire;
  };


  struct Session
  {
    SessionToken token;
    CacheMap map;
    ExpireInfo expireInfo;

    bool shared{false};
    bool expires{false};    
  };


  struct ExpiryTracking
  {
    SessionToken token;
    SessionExpireTime time{};
    bool deleteOnExpire{true};
    SessionDuration duration{};
  };


public:
  using SessionType = Session;
  using SessionsMap = ankerl::unordered_dense::segmented_map<SessionToken, Session>;


public:

  std::optional<std::reference_wrapper<CacheMap>> start (const SessionToken& token, const bool shared, const SessionDuration duration, const bool deleteOnExpire)
  {
    if (m_sessions.contains(token))
      return {};  // TODO check this, if shared:true and a session with this name already exists
    else
    {
      if (duration == SessionDuration::zero())  // if never expires
        m_sessions[token] = Session{.token = token, .shared = shared, .expires = false};
      else
      {
        auto expireTime = SessionClock::now() + duration;

        ExpireInfo expire {.time = expireTime, .duration = duration, .deleteOnExpire = deleteOnExpire};
        m_sessions[token] = Session{.token = token, .expireInfo = std::move(expire), .shared = shared, .expires = true};

        m_expiry.emplace(std::make_pair(expireTime, ExpiryTracking{.token = token, .time = expireTime, .deleteOnExpire = deleteOnExpire, .duration = duration}));
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
    if (auto sesh = m_sessions.find(token); sesh != m_sessions.end())
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
      }

      m_sessions.erase(sesh);
    }
    
    return ended;
  }

  
  std::size_t clear ()
  {
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
    if (auto it = m_sessions.find(token) ; it == m_sessions.end())
      return {};
    else
      return {it->second};
  }


  void handleExpired (const SessionExpireTime time = SessionClock::now())
  {    
    const auto itExpired = m_expiry.lower_bound(time);

    if (itExpired != m_expiry.end() || (itExpired == m_expiry.end() && !m_expiry.empty()))
    {
      for (auto it = m_expiry.cbegin() ; it != itExpired; )
      {
        if (it->second.deleteOnExpire)
        {
          m_sessions.erase(it->second.token);
          it = m_expiry.erase(it);
        }
        else if (m_sessions.contains(it->second.token)) // should always be true because end() removes the entry, but sanity check
        {
          // session expired but not deleted, so clear data and reset expiry
          auto& expireInfo = it->second;
          auto& session = m_sessions.at(expireInfo.token);

          const SessionExpireTime expireTime = SessionClock::now() + expireInfo.duration;

          auto node = m_expiry.extract(it);

          node.key() = expireTime;
          node.mapped().time = expireTime;

          m_expiry.insert(std::move(node));

          session.map.clear();
          session.expireInfo.time = expireTime;

          ++it;
        }
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

        const auto expireTime = SessionClock::now() + node.mapped().duration;

        sesh.expireInfo.time = expireTime;
        
        node.mapped().time = expireTime;
        node.key() = expireTime;

        m_expiry.insert(std::move(node));
      }
    }
  }


private:
  SessionsMap m_sessions;
  std::multimap<SessionExpireTime, ExpiryTracking> m_expiry; // TODO consider priority queue
};


}
}
}

#endif
