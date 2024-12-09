#ifndef NDB_CORE_ARRCOMMANDS_H
#define NDB_CORE_ARRCOMMANDS_H

#include <fixed_string.hpp>

namespace nemesis { namespace arr { namespace cmds {

  using namespace fixstr;

  template<std::size_t N>
  using FixedString = fixstr::fixed_string<N>;


  // const char CreateReq[]      = "ARR_CREATE";  
  // const char CreateRsp[]      = "ARR_CREATE_RSP";
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
  // const char SetReq[]     = "ARR_SET";  
  // const char SetRsp[]     = "ARR_SET_RSP";
  const char SetRngReq[]  = "ARR_SET_RNG";  
  const char SetRngRsp[]  = "ARR_SET_RNG_RSP";
  // const char GetReq[]     = "ARR_GET";  
  // const char GetRsp[]     = "ARR_GET_RSP";
  const char GetRngReq[]  = "ARR_GET_RNG";  
  const char GetRngRsp[]  = "ARR_GET_RNG_RSP";


  static constexpr FixedString Rsp        = "_RSP";  

  static constexpr FixedString Create     = "CREATE";  
  static constexpr FixedString Delete     = "DELETE";
  static constexpr FixedString DeleteAll  = "DELETE_ALL";
  static constexpr FixedString Exist      = "EXIST";
  static constexpr FixedString Set        = "SET";  
  static constexpr FixedString SetRng     = "SET_RNG";  
  static constexpr FixedString Get        = "GET";  
  static constexpr FixedString GetRng     = "GET_RNG";  
  static constexpr FixedString Len        = "LEN";
  static constexpr FixedString Swap       = "SWAP";
  static constexpr FixedString Clear      = "CLEAR";
  

  template<FixedString Ident, FixedString Cmd>
  static consteval auto makeReq() -> decltype(Ident+Cmd)
  {
    return Ident+Cmd;
  }

  template<FixedString Ident, FixedString Cmd>
  static consteval auto makeRsp() -> decltype(Ident+Cmd+Rsp)
  {
    return Ident+Cmd+Rsp;
  }


  // OARR
  static constexpr FixedString OArrayIdent   = "OARR";
  static constexpr FixedString OArrayIdent_  = "OARR_";
  
  template<FixedString C>
  static consteval auto oArrayReq() -> decltype(OArrayIdent_ + C)
  {
    return makeReq<OArrayIdent_, C>();
  }

  template<FixedString C>
  static consteval auto oArrayRsp() -> decltype(OArrayIdent_+C+Rsp)
  {
    return makeRsp<OArrayIdent_, C>();
  }

  
  struct OArrCmds
  {   
    static constexpr auto CreateReq     = oArrayReq<Create>();
    static constexpr auto CreateRsp     = oArrayRsp<Create>();
    static constexpr auto SetReq        = oArrayReq<Set>();
    static constexpr auto SetRsp        = oArrayRsp<Set>();
    static constexpr auto DeleteReq     = oArrayReq<Delete>();
    static constexpr auto DeleteRsp     = oArrayRsp<Delete>();
    static constexpr auto DeleteAllReq  = oArrayReq<DeleteAll>();
    static constexpr auto DeleteAllRsp  = oArrayRsp<DeleteAll>();
    static constexpr auto ExistReq      = oArrayReq<Exist>();
    static constexpr auto ExistRsp      = oArrayRsp<Exist>();
    static constexpr auto LenReq        = oArrayReq<Len>();
    static constexpr auto LenRsp        = oArrayRsp<Len>();
    static constexpr auto SwapReq       = oArrayReq<Swap>();
    static constexpr auto SwapRsp       = oArrayRsp<Swap>();
    static constexpr auto ClearReq      = oArrayReq<Clear>();
    static constexpr auto ClearRsp      = oArrayRsp<Clear>();
    static constexpr auto SetRngReq     = oArrayReq<SetRng>();
    static constexpr auto SetRngRsp     = oArrayRsp<SetRng>();
    static constexpr auto GetReq        = oArrayReq<Get>();
    static constexpr auto GetRsp        = oArrayRsp<Get>();
    static constexpr auto GetRngReq     = oArrayReq<GetRng>();
    static constexpr auto GetRngRsp     = oArrayRsp<GetRng>();
  };

}
}
}

#endif
