#ifndef _FC_SHSESSIONS_
#define _FC_SHSESSIONS_

#include <string>
#include <string_view>
#include <chrono>
#include <ankerl/unordered_dense.h>
#include <core/FusionCommon.h>
#include <core/CacheMap.h>
#include <core/kv/KvCommon.h>


namespace fusion { namespace core { namespace kv {

using namespace fusion::core;


class Sessions
{
private:
  struct Session
  {
    SessionToken token;
    CacheMap map;
  };

  using SessionsMap = ankerl::unordered_dense::segmented_map<SessionToken, Session>;


public:

  std::optional<std::reference_wrapper<CacheMap>> start (const SessionToken& token)
  {
    if (m_sessions.contains(token))
      return {};

    m_sessions[token] = Session{.token = token};
    return m_sessions.at(token).map;
  }


  bool end (const SessionToken& token)
  {
    return m_sessions.erase(token) != 0U;
  }


  std::optional<std::reference_wrapper<CacheMap>> get (const SessionToken& token)
  {
    if (auto it = m_sessions.find(token) ; it == m_sessions.end())
      return {};
    else
      return {it->second.map};
  }


private:

  fc_always_inline void send (KvCommand& cmd, std::string&& msg)
  {
    cmd.loop->defer([cmd, msg = std::move(msg)] () mutable
    {
      if (cmd.ws->getUserData()->connected)
        cmd.ws->send(msg, kv::WsSendOpCode);
    });
  }
  

private:
  SessionsMap m_sessions;  
};


}
}
}

#endif
