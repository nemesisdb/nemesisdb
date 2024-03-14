#ifndef _FC_CORESESSIONEXECUTOR_
#define _FC_CORESESSIONEXECUTOR_

#include <functional>
#include <core/NemesisCommon.h>
#include <core/kv/KvCommon.h>
#include <core/kv/KvSessions.h>


namespace nemesis { namespace core { namespace kv {

using namespace nemesis::core;
using namespace nemesis::core::kv;


enum class SessionExecutorType 
{
  ShNew
};

enum class KvExecutorType
{
  KvSetQ,
  KvGet
};


class SessionExecutor
{
public:
  using Handler = std::function<njson(Sessions&, const SessionToken&, njson&)> ;


  static Handler create(const SessionExecutorType t)
  {
    switch (t)
    {
      using enum SessionExecutorType;

      case ShNew:
        return newSession;
        break;
      
      default:
        // below
        break;
    }

    assert("SessionExecutor::create() - KvQueryType not recognised");
    return nullptr;
  }


private:
  static njson newSession (Sessions& sessions, const SessionToken& tkn, njson& cmd)
  {
    const SessionDuration duration {cmd.at("expiry").at("duration").as<SessionDuration::rep>()};
    const bool deleteOnExpire = cmd.at("expiry").at("deleteSession").as_bool();

    if (const auto cache = sessions.start(tkn, cmd.at("shared") == true, duration, deleteOnExpire); cache)
      return PoolRequestResponse::sessionNew(RequestStatus::Ok, tkn, cmd.at("name").as_string());
    else
      return PoolRequestResponse::sessionNew(RequestStatus::SessionNewFail, tkn, cmd.at("name").as_string());
  }
};


class KvExecutor
{
public:
  using Handler = std::function<njson(CacheMap&, const SessionToken&, njson&)> ;


  static Handler create(const KvExecutorType t)
  {
    switch (t)
    {
      using enum KvExecutorType;

      case KvSetQ:
        return setQ;
        break;
      
      case KvGet:
        return get;
        break;

      default:
        // below
        break;
    }

    assert("SessionExecutor::create() - KvQueryType not recognised");
    return nullptr;
  }


private:
  static njson setQ (CacheMap& map, const SessionToken& tkn, njson& cmd)
  {
    njson rsp = njson::null();
      
    for(auto& kv : cmd.object_range())
    {
      try
      {
        map.set(kv.key(), std::move(kv.value()));
      }
      catch(const std::exception& e)
      {
        rsp["KV_SETQ_RSP"]["keys"][kv.key()] = toUnderlying(RequestStatus::Unknown);
      }
    }

    if (!rsp.is_null())
      rsp["KV_SETQ_RSP"]["tkn"] = tkn;

    return rsp;
  }


  static njson get (CacheMap& map, const SessionToken& tkn, njson& cmd)
  {
    njson rsp;
    rsp["KV_GET_RSP"]["tkn"] = tkn;

    for(auto& item : cmd.array_range())
    {
      if (item.is_string()) [[likely]]
      {
        const auto& key = item.as_string();
        if (auto [exists, value] = map.get(key); exists)
          rsp["KV_GET_RSP"]["keys"][key] = std::move(value);
        else
          rsp["KV_GET_RSP"]["keys"][key] = njson::null();
      }
    }

    return rsp;
  }

};


}
}
}

#endif

