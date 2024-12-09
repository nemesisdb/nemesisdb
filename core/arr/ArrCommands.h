#ifndef NDB_CORE_ARRCOMMANDS_H
#define NDB_CORE_ARRCOMMANDS_H

#include <fixed_string.hpp>

namespace nemesis { namespace arr { namespace cmds {

  using namespace fixstr;

  //const char ArrIdent[] = "ARR";

  const char CreateReq[]      = "ARR_CREATE";  
  const char CreateRsp[]      = "ARR_CREATE_RSP";
  const char DeleteReq[]      = "ARR_DELETE";
  const char DeleteRsp[]      = "ARR_DELETE_RSP";
  const char DeleteAllReq[]   = "ARR_DELETE_ALL";
  const char DeleteAllRsp[]   = "ARR_DELETE_ALL_RSP";
  const char ExistReq[]       = "ARR_EXIST";
  const char ExistRsp[]       = "ARR_EXIST_RSP";
  const char LenReq[]         = "ARR_LEN";
  const char LenRsp[]         = "ARR_LEN_RSP";
  const char SwapReq[]    = "ARR_SWAP";
  const char SwapRsp[]    = "ARR_SWAP_RSP";
  const char ClearReq[]   = "ARR_CLEAR";
  const char ClearRsp[]   = "ARR_CLEAR_RSP";  
  const char SetReq[]     = "ARR_SET";  
  const char SetRsp[]     = "ARR_SET_RSP";
  const char SetRngReq[]  = "ARR_SET_RNG";  
  const char SetRngRsp[]  = "ARR_SET_RNG_RSP";
  const char GetReq[]     = "ARR_GET";  
  const char GetRsp[]     = "ARR_GET_RSP";
  const char GetRngReq[]  = "ARR_GET_RNG";  
  const char GetRngRsp[]  = "ARR_GET_RNG_RSP";


  static constexpr fixstr::fixed_string Rsp     = "_RSP";  
  static constexpr fixstr::fixed_string Create  = "CREATE";
  static constexpr fixstr::fixed_string Set     = "SET";

  
  template<fixstr::fixed_string Ident, fixstr::fixed_string Cmd>
  static consteval auto makeReq() -> decltype(Ident+Cmd)
  {
    return Ident+Cmd;
  }

  template<fixstr::fixed_string Ident, fixstr::fixed_string Cmd>
  static consteval auto makeRsp() -> decltype(Ident+Cmd+Rsp)
  {
    return Ident+Cmd+Rsp;
  }


  struct OArrCmds
  { 
    public:
      static constexpr fixstr::fixed_string Ident   = "ARR";

    private:
      static constexpr fixstr::fixed_string Ident_  = "ARR_";

      
      // template<fixstr::fixed_string C, fixstr::fixed_string I = Ident_>
      // static consteval auto req() -> decltype(makeReq<Ident_,C>())
      // {
      //   return makeReq<I, C>();
      // }
      // ..
      // this produces "used before its definition" error, don't know why
      //static constexpr auto CreateReq = req<Create>();

    public:
      static constexpr auto CreateReq = makeReq<Ident_, Create>();
      static constexpr auto CreateRsp = makeRsp<Ident_, Create>();

      static constexpr auto SetReq = makeReq<Ident_, Set>();
      static constexpr auto SetRsp = makeRsp<Ident_, Set>();
    
  };

}
}
}

#endif
