#ifndef NDB_CORE_LSTCOMMANDS_H
#define NDB_CORE_LSTCOMMANDS_H

#include <core/NemesisCommon.h>

namespace nemesis { namespace lst { namespace cmds {

  //
  // For now, there is only an Object List. For
  // ints or strings, there are arrays

  static constexpr FixedString Rsp        = "_RSP";  

  static constexpr FixedString Create     = "CREATE";  
  static constexpr FixedString Delete     = "DELETE";
  static constexpr FixedString DeleteAll  = "DELETE_ALL";
  static constexpr FixedString Exist      = "EXIST";
  static constexpr FixedString Add        = "ADD";
  static constexpr FixedString SetRng     = "SET_RNG";
  static constexpr FixedString Get        = "GET";
  static constexpr FixedString GetRng     = "GET_RNG";
  static constexpr FixedString Len        = "LEN";
  static constexpr FixedString Swap       = "SWAP";
  static constexpr FixedString Clear      = "CLEAR";
  static constexpr FixedString Intersect  = "INTERSECT";
  

  template<FixedString Ident, FixedString Cmd>
  static consteval auto makeReq() -> decltype(Ident+Cmd)
  {
    return Ident+Cmd;
  }

  template<FixedString Ident, FixedString Cmd>
  static consteval auto makeRsp() -> decltype(makeReq<Ident, Cmd>()+Rsp)
  {
    return makeReq<Ident, Cmd>()+Rsp;
  }


  static constexpr FixedString ListIdent     = "OLST";
  static constexpr FixedString ListIdent_    = "OLST_";


  template <FixedString Ident>
  struct LstCmds
  {
    static constexpr auto CreateReq     = makeReq<Ident,Create>();
    static constexpr auto CreateRsp     = makeRsp<Ident,Create>();
    static constexpr auto DeleteReq     = makeReq<Ident,Delete>();
    static constexpr auto DeleteRsp     = makeRsp<Ident,Delete>();
    static constexpr auto DeleteAllReq  = makeReq<Ident,DeleteAll>();
    static constexpr auto DeleteAllRsp  = makeRsp<Ident,DeleteAll>();
    static constexpr auto ExistReq      = makeReq<Ident,Exist>();
    static constexpr auto ExistRsp      = makeRsp<Ident,Exist>();
    static constexpr auto LenReq        = makeReq<Ident,Len>();
    static constexpr auto LenRsp        = makeRsp<Ident,Len>();
    static constexpr auto AddReq        = makeReq<Ident,Add>();
    static constexpr auto AddRsp        = makeRsp<Ident,Add>();
    static constexpr auto SetRngReq     = makeReq<Ident,SetRng>();
    static constexpr auto SetRngRsp     = makeRsp<Ident,SetRng>();
    static constexpr auto GetReq        = makeReq<Ident,Get>();
    static constexpr auto GetRsp        = makeRsp<Ident,Get>();
    static constexpr auto GetRngReq     = makeReq<Ident,GetRng>();
    static constexpr auto GetRngRsp     = makeRsp<Ident,GetRng>();
    static constexpr auto ClearReq      = makeReq<Ident,Clear>();
    static constexpr auto ClearRsp      = makeRsp<Ident,Clear>();
    static constexpr auto SwapReq       = makeReq<Ident,Swap>();
    static constexpr auto SwapRsp       = makeRsp<Ident,Swap>();
    static constexpr auto IntersectReq  = makeReq<Ident,Intersect>();    
    static constexpr auto IntersectRsp  = makeRsp<Ident,Intersect>();
  };

  
  template <typename T, JsonType JT, FixedString Ident>
  struct UnsortedList : public LstCmds<Ident>
  {
    using ItemT = T;
    static constexpr JsonType ItemJsonT = JT;
  };
  

  // Object List
  struct ListCmds : public UnsortedList<njson, JsonObject, ListIdent_>
  {  
    static constexpr bool isTypeValid (const JsonType t)
    {
      return t == ItemJsonT;
    }
  };

}
}
}

#endif
