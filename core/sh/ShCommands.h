#ifndef NDB_CORE_SHCOMMANDS_H
#define NDB_CORE_SHCOMMANDS_H


namespace nemesis { namespace sh { namespace cmds {

  const char ShIdent[] = "SH";

  const char NewReq[]       = "SH_NEW";  
  const char NewRsp[]       = "SH_NEW_RSP";
  const char EndReq[]       = "SH_END";
  const char EndRsp[]       = "SH_END_RSP";
  const char InfoReq[]      = "SH_INFO";
  const char InfoRsp[]      = "SH_INFO_RSP";
  const char InfoAllReq[]   = "SH_INFO_ALL";
  const char InfoAllRsp[]   = "SH_INFO_ALL_RSP";
  const char SaveReq[]      = "SH_SAVE";
  const char SaveRsp[]      = "SH_SAVE_RSP";
  const char LoadReq[]      = "SH_LOAD";
  const char LoadRsp[]      = "SH_LOAD_RSP";
  const char EndAllReq[]    = "SH_END_ALL";
  const char EndAllRsp[]    = "SH_END_ALL_RSP";
  const char ExistsReq[]    = "SH_EXISTS";
  const char ExistsRsp[]    = "SH_EXISTS_RSP";
  //
  const char SetReq[]       = "SH_SET";
  const char SetRsp[]       = "SH_SET_RSP";
  const char GetReq[]       = "SH_GET";
  const char GetRsp[]       = "SH_GET_RSP";
  const char AddReq[]       = "SH_ADD";
  const char AddRsp[]       = "SH_ADD_RSP";
  const char RmvReq[]       = "SH_RMV";
  const char RmvRsp[]       = "SH_RMV_RSP";
  const char ClearReq[]     = "SH_CLEAR";
  const char ClearRsp[]     = "SH_CLEAR_RSP";
  const char CountReq[]     = "SH_COUNT";
  const char CountRsp[]     = "SH_COUNT_RSP";
  const char ContainsReq[]    = "SH_CONTAINS";
  const char ContainsRsp[]    = "SH_CONTAINS_RSP";
  const char KeysReq[]        = "SH_KEYS";
  const char KeysRsp[]        = "SH_KEYS_RSP";
  const char ClearSetReq[]    = "SH_CLEAR_SET";
  const char ClearSetRsp[]    = "SH_CLEAR_SET_RSP";

}
}
}

#endif
