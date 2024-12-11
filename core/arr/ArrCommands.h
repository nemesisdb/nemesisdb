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
  static constexpr FixedString Intersect  = "INTERSECT";
  

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


  static constexpr FixedString OArrayIdent   = "OARR";
  static constexpr FixedString OArrayIdent_  = "OARR_";

  static constexpr FixedString IntArrayIdent   = "IARR";
  static constexpr FixedString IntArrayIdent_  = "IARR_";

  static constexpr FixedString StrArrayIdent   = "STRARR";
  static constexpr FixedString StrArrayIdent_  = "STRARR_";
  
  static constexpr FixedString SortedIntArrayIdent   = "SIARR";
  static constexpr FixedString SortedIntArrayIdent_  = "SIARR_";

  static constexpr FixedString SortedStrArrayIdent   = "SSTRARR"; // Sorted STRing ARRay
  static constexpr FixedString SortedStrArrayIdent_  = "SSTRARR_";
  

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
    static constexpr auto ClearReq      = makeReq<Ident,Clear>();
    static constexpr auto ClearRsp      = makeRsp<Ident,Clear>();
    static constexpr auto SetRngReq     = makeReq<Ident,SetRng>();
    static constexpr auto SetRngRsp     = makeRsp<Ident,SetRng>();
    static constexpr auto GetReq        = makeReq<Ident,Get>();
    static constexpr auto GetRsp        = makeRsp<Ident,Get>();
    static constexpr auto GetRngReq     = makeReq<Ident,GetRng>();
    static constexpr auto GetRngRsp     = makeRsp<Ident,GetRng>();
    
    // not enabled in sorted containers
    static constexpr auto SwapReq       = makeReq<Ident,Swap>();
    static constexpr auto SwapRsp       = makeRsp<Ident,Swap>();
    
    // only enabled in sorted containers
    static constexpr auto IntersectReq = makeReq<Ident,Intersect>();
    static constexpr auto IntersectRsp = makeRsp<Ident,Intersect>();
  };

  
  //
  // These structs build the command names by passing the ident
  // required. 
  // The IsSorted and CanIntersect are both probably not required,
  // because intersection requires a sorted container, so can probably
  // remove CanIntersect, unless there's a container that's sorted but
  // can't be intersected. Decide when linked list is implemented.
  //

  // Object Array
  struct OArrCmds : public ArrCmds<OArrayIdent_>
  {   
    using ItemT = njson;
    
    static constexpr JsonType ItemJsonT = JsonObject;
    static constexpr bool IsSorted = false;
    static constexpr bool CanIntersect = false;

    static constexpr bool isTypeValid (const JsonType t)
    {
      return t == ItemJsonT;
    }
  };


  // Integer Array (signed)
  struct IntArrCmds : public ArrCmds<IntArrayIdent_>
  {   
    using ItemT = std::int64_t;
    
    static constexpr JsonType ItemJsonT = JsonInt;
    static constexpr bool IsSorted = false;
    static constexpr bool CanIntersect = false;

    static constexpr bool isTypeValid (const JsonType t)
    {
      // jsoncons (and possibly JSON specs) stores an an integer 
      // as unsigned unless the value is negative, then its signed int.
      // this containres handles both, but could create
      // an unsigned version if required.
      return t == JsonUInt || t == JsonInt;
    }
  };


  // String Array
  struct StrArrCmds : public ArrCmds<StrArrayIdent_>
  {   
    using ItemT = std::string;
    
    static constexpr JsonType ItemJsonT = JsonString;
    static constexpr bool IsSorted = true;
    static constexpr bool CanIntersect = false;

    static constexpr bool isTypeValid (const JsonType t)
    {
      return t == ItemJsonT;
    }
  };


  // Sorted Integer Array (signed)
  struct SortedIntArrCmds : public ArrCmds<SortedIntArrayIdent_>
  {   
    using ItemT = std::int64_t;
    
    static constexpr JsonType ItemJsonT = JsonInt;
    static constexpr bool IsSorted = true;
    static constexpr bool CanIntersect = true;

    static constexpr bool isTypeValid (const JsonType t)
    {
      return t == JsonUInt || t == JsonInt;
    }
  };


  // Sorted String Array
  struct SortedStrArrCmds : public ArrCmds<SortedStrArrayIdent_>
  {   
    using ItemT = std::string;
    
    static constexpr JsonType ItemJsonT = JsonString;
    static constexpr bool IsSorted = true;
    static constexpr bool CanIntersect = true;

    static constexpr bool isTypeValid (const JsonType t)
    {
      return t == ItemJsonT;
    }
  };
}
}
}

#endif
