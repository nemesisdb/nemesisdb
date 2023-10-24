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
  };


  struct ExpiryTrackingCmp
  {
    bool operator()(const ExpiryTracking l, const ExpiryTracking& r)
    {
      return l.time > r.time;
    }

  };


  using SessionsMap = ankerl::unordered_dense::segmented_map<SessionToken, Session>;

public:

  std::optional<std::reference_wrapper<CacheMap>> start (const SessionToken& token, const bool shared, const SessionDuration duration, const bool deleteOnExpire)
  {
    if (auto seshIt = m_sessions.find(token) ; seshIt != m_sessions.cend())
      return seshIt->second.map;
    else
    {
      if (duration != SessionDuration::zero())
      {
        auto expireTime = SessionClock::now() + duration;

        ExpireInfo expire {.time = expireTime, .duration = duration, .deleteOnExpire = deleteOnExpire};
        m_sessions[token] = Session{.token = token, .expireInfo = std::move(expire), .shared = shared, .expires = true};

        m_expiry.emplace(std::make_pair(expireTime, ExpiryTracking{.token = token, .time = expireTime, .deleteOnExpire = deleteOnExpire}));
      }
      else
        m_sessions[token] = Session{.token = token, .shared = shared, .expires = false};

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
    if (auto sesh = m_sessions.find(token); sesh != m_sessions.end())
    {
      m_expiry.erase(sesh->second.expireInfo.time);
      return m_sessions.erase(token) != 0U;
    }

    return false;
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


  std::optional<std::reference_wrapper<const Session>> get (const SessionToken& token) const
  {
    if (auto it = m_sessions.find(token) ; it == m_sessions.end())
      return {};
    else
      return {it->second};
  }


  void removeExpired (const SessionExpireTime time = SessionClock::now())
  {
    auto itExpired = m_expiry.lower_bound(time);

    if (itExpired != m_expiry.end() || (itExpired == m_expiry.end() && !m_expiry.empty()))
    {
      for (auto it = m_expiry.cbegin() ; it != itExpired; ++it)
      {
        if (it->second.deleteOnExpire)
          m_sessions.erase(it->second.token);
        else if (m_sessions.contains(it->second.token)) // shouldn't happen because end() removes the entry, but sanity check
          m_sessions.at(it->second.token).map.clear();
      }

      m_expiry.erase(m_expiry.begin(), itExpired);
    }
  }


private:
  SessionsMap m_sessions;
  std::multimap<SessionExpireTime, ExpiryTracking> m_expiry;
};


}
}
}

#endif
