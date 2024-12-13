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
  static constexpr FixedString Used       = "USED";
  static constexpr FixedString Swap       = "SWAP";
  static constexpr FixedString Clear      = "CLEAR";
  static constexpr FixedString Intersect  = "INTERSECT";
  static constexpr FixedString Min        = "MIN";
  static constexpr FixedString Max        = "MAX";
  

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
    static constexpr auto UsedReq       = makeReq<Ident,Used>();
    static constexpr auto UsedRsp       = makeRsp<Ident,Used>();
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
    static constexpr auto MinReq = makeReq<Ident,Min>();
    static constexpr auto MinRsp = makeRsp<Ident,Min>();
    static constexpr auto MaxReq = makeReq<Ident,Max>();
    static constexpr auto MaxRsp = makeRsp<Ident,Max>();
  };

  
  template <typename T, JsonType JT, FixedString Ident>
  struct UnsortedArray : public ArrCmds<Ident>
  {
    using ItemT = T;
    
    static constexpr JsonType ItemJsonT = JT;
    static constexpr bool IsSorted = false;
    static constexpr bool CanIntersect = false;
  };


  //
  // These structs build the command names by passing the ident
  // required.   
  //

  // Object Array
  struct OArrCmds : public UnsortedArray<njson, JsonObject, OArrayIdent_>
  {  
    static constexpr bool isTypeValid (const JsonType t)
    {
      return t == ItemJsonT;
    }
  };


  // Integer Array (signed)
  struct IntArrCmds : public UnsortedArray<std::int64_t, JsonInt, IntArrayIdent_>
  {  
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
  struct StrArrCmds : public UnsortedArray<std::string, JsonString, StrArrayIdent_>
  {   
    static constexpr bool isTypeValid (const JsonType t)
    {
      return t == ItemJsonT;
    }
  };


  // The IsSorted and CanIntersect are both probably not required,
  // because intersection requires a sorted container, so can probably
  // remove CanIntersect. There may be a container that's sorted but
  // can't be intersected. Decide when linked list is implemented.

  template <typename T, JsonType JT, FixedString Ident>
  struct SortedArray : public ArrCmds<Ident>
  {
    using ItemT = T;
    
    static constexpr JsonType ItemJsonT = JT;
    static constexpr bool IsSorted = true;
    static constexpr bool CanIntersect = true;
  };


  // Sorted Integer Array (signed)
  struct SortedIntArrCmds : public SortedArray<std::int64_t, JsonInt, SortedIntArrayIdent_>
  {   
    static constexpr bool isTypeValid (const JsonType t)
    {
      return t == JsonUInt || t == JsonInt;
    }
  };


  // Sorted String Array
  struct SortedStrArrCmds : public SortedArray<std::string, JsonString, SortedStrArrayIdent_>
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
