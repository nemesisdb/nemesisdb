#ifndef NDB_CORE_ARRCOMMANDS_H
#define NDB_CORE_ARRCOMMANDS_H


namespace nemesis { namespace arr { namespace cmds {

  const char ArrIdent[] = "ARR";

  const char CreateReq[]  = "ARR_CREATE";  
  const char CreateRsp[]  = "ARR_CREATE_RSP";
  const char DeleteReq[]  = "ARR_DELETE";
  const char DeleteRsp[]  = "ARR_DELETE_RSP";
  const char SetReq[]     = "ARR_SET";  
  const char SetRsp[]     = "ARR_SET_RSP";
  const char GetReq[]     = "ARR_GET";  
  const char GetRsp[]     = "ARR_GET_RSP";
  const char GetRngReq[]  = "ARR_GET_RNG";  
  const char GetRngRsp[]  = "ARR_GET_RNG_RSP";
}
}
}

#endif
