#ifndef NDB_CORE_KVCOMMANDS_H
#define NDB_CORE_KVCOMMANDS_H


namespace nemesis { namespace kv { namespace cmds {

  const char KvIdent[] = "KV";

  const char SetReq[]     = "KV_SET";
  const char SetRsp[]     = "KV_SET_RSP";
  const char GetReq[]     = "KV_GET";
  const char GetRsp[]     = "KV_GET_RSP";
  const char AddReq[]     = "KV_ADD";
  const char AddRsp[]     = "KV_ADD_RSP";
  const char RmvReq[]     = "KV_RMV";
  const char RmvRsp[]     = "KV_RMV_RSP";
  const char ClearReq[]   = "KV_CLEAR";
  const char ClearRsp[]   = "KV_CLEAR_RSP";
  const char CountReq[]   = "KV_COUNT";
  const char CountRsp[]   = "KV_COUNT_RSP";
  const char ContainsReq[]    = "KV_CONTAINS";
  const char ContainsRsp[]    = "KV_CONTAINS_RSP";
  const char KeysReq[]        = "KV_KEYS";
  const char KeysRsp[]        = "KV_KEYS_RSP";
  const char ClearSetReq[]    = "KV_CLEAR_SET";
  const char ClearSetRsp[]    = "KV_CLEAR_SET_RSP";
  const char SaveReq[]        = "KV_SAVE";
  const char SaveRsp[]        = "KV_SAVE_RSP";
  const char LoadReq[]        = "KV_LOAD";
  const char LoadRsp[]        = "KV_LOAD_RSP";
}
}
}

#endif
