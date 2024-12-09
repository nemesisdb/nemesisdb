#ifndef NDB_CORE_ARRCOMMANDS_H
#define NDB_CORE_ARRCOMMANDS_H

#include <fixed_string.hpp>

namespace nemesis { namespace arr { namespace cmds {

  using namespace fixstr;

  template<std::size_t N>
  using FixedString = fixstr::fixed_string<N>;


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

  static constexpr FixedString IntArrayIdent   = "IARR";
  static constexpr FixedString IntArrayIdent_  = "IARR_";

  static constexpr FixedString StrArrayIdent   = "SARR";
  static constexpr FixedString StrArrayIdent_  = "SARR_";
  

  template <FixedString Ident>
  struct ArrCmds
  {
    static constexpr auto CreateReq     = makeReq<Ident,Create>();
    static constexpr auto CreateRsp     = makeRsp<Ident,Create>();
    static constexpr auto SetReq        = makeReq<Ident,Set>();
    static constexpr auto SetRsp        = makeRsp<Ident,Set>();
    static constexpr auto DeleteReq     = makeReq<Ident,Delete>();
    static constexpr auto DeleteRsp     = makeRsp<Ident,Delete>();
    static constexpr auto DeleteAllReq  = makeReq<Ident,DeleteAll>();
    static constexpr auto DeleteAllRsp  = makeRsp<Ident,DeleteAll>();
    static constexpr auto ExistReq      = makeReq<Ident,Exist>();
    static constexpr auto ExistRsp      = makeRsp<Ident,Exist>();
    static constexpr auto LenReq        = makeReq<Ident,Len>();
    static constexpr auto LenRsp        = makeRsp<Ident,Len>();
    static constexpr auto SwapReq       = makeReq<Ident,Swap>();
    static constexpr auto SwapRsp       = makeRsp<Ident,Swap>();
    static constexpr auto ClearReq      = makeReq<Ident,Clear>();
    static constexpr auto ClearRsp      = makeRsp<Ident,Clear>();
    static constexpr auto SetRngReq     = makeReq<Ident,SetRng>();
    static constexpr auto SetRngRsp     = makeRsp<Ident,SetRng>();
    static constexpr auto GetReq        = makeReq<Ident,Get>();
    static constexpr auto GetRsp        = makeRsp<Ident,Get>();
    static constexpr auto GetRngReq     = makeReq<Ident,GetRng>();
    static constexpr auto GetRngRsp     = makeRsp<Ident,GetRng>();
  };

  
  struct OArrCmds : public ArrCmds<OArrayIdent_>
  {   
    using ItemT = njson;
    
    static constexpr JsonType ItemJsonT = JsonObject;

    static constexpr bool isTypeValid (const JsonType t)
    {
      return t == ItemJsonT;
    }
  };


  struct IntArrCmds : public ArrCmds<IntArrayIdent_>
  {   
    using ItemT = std::int64_t;
    
    static constexpr JsonType ItemJsonT = JsonInt;

    static constexpr bool isTypeValid (const JsonType t)
    {
      return t == JsonUInt || t == JsonInt;
    }
  };


  struct StrArrCmds : public ArrCmds<StrArrayIdent_>
  {   
    using ItemT = std::string;
    
    static constexpr JsonType ItemJsonT = JsonString;

    static constexpr bool isTypeValid (const JsonType t)
    {
      return t == JsonString;
    }
  };

}
}
}

#endif
