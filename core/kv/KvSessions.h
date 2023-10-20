#ifndef _FC_SHSESSIONS_
#define _FC_SHSESSIONS_

#include <string>
#include <string_view>
#include <chrono>
#include <map>
#include <ankerl/unordered_dense.h>
#include <core/FusionCommon.h>
#include <core/CacheMap.h>
#include <core/kv/KvCommon.h>


namespace fusion { namespace core { namespace kv {

using namespace fusion::core;


class Sessions
{
private:
  struct ExpireInfo
  {
    SessionExpireTime time;
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

  std::optional<std::reference_wrapper<CacheMap>> start (const SessionToken& token, const bool shared, const SessionDuration duration)
  {
    if (auto seshIt = m_sessions.find(token) ; seshIt != m_sessions.cend())
      return seshIt->second.map;
    else
    {
      if (duration != SessionDuration::zero())
      {
        auto expireTime = SessionClock::now() + duration;
        ExpireInfo expire {.time = expireTime};
        m_sessions[token] = Session{.token = token, .expireInfo = std::move(expire), .shared = shared, .expires = true};

        m_expiry.emplace(std::make_pair(expireTime, ExpiryTracking{.token = token, .time = expireTime}));
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


  bool end (const SessionToken& token)
  {
    return m_sessions.erase(token) != 0U;
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
      std::cout << std::distance(m_expiry.begin(), itExpired) << " sessions expired\n";

      for (auto it = m_expiry.cbegin() ; it != itExpired; ++it)
        m_sessions.erase(it->second.token);

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
