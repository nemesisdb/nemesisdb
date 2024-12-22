#ifndef NDB_CORE_LSTCOMMANDS_H
#define NDB_CORE_LSTCOMMANDS_H

#include <core/NemesisCommon.h>

namespace nemesis { namespace lst { namespace cmds {

  //
  // For now, there is only an Object List. For
  // ints or strings, there are arrays

  static constexpr FixedString Create     = "CREATE";  
  static constexpr FixedString Delete     = "DELETE";
  static constexpr FixedString DeleteAll  = "DELETE_ALL";
  static constexpr FixedString Exist      = "EXIST";
  static constexpr FixedString Add        = "ADD";
  static constexpr FixedString SetRng     = "SET_RNG";
  static constexpr FixedString Get        = "GET";
  static constexpr FixedString GetRng     = "GET_RNG";
  static constexpr FixedString Len        = "LEN";
  static constexpr FixedString Splice     = "SPLICE";
  static constexpr FixedString Remove     = "RMV";
  static constexpr FixedString Intersect  = "INTERSECT";
  

  static constexpr FixedString ListIdent   = "OLST";
  static constexpr FixedString ListIdent_  = "OLST_";
  static constexpr FixedString Rsp         = "_RSP";  


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


  template <FixedString Ident>
  struct LstCmds
  {
    template<FixedString Name>
    struct Cmd
    {
      static constexpr auto req = makeReq<Ident, Name>();
      static constexpr auto rsp = makeRsp<Ident, Name>();
    };

    // TODO do for Arrays:
    static constexpr Cmd<Create> create {};    
    static constexpr Cmd<Delete> del{}; // or delete_ ?
    static constexpr Cmd<DeleteAll> deleteAll{}; 
    static constexpr Cmd<Exist> exist{};
    static constexpr Cmd<Len> len{};
    static constexpr Cmd<Add> add {};
    static constexpr Cmd<SetRng> setRng{};
    static constexpr Cmd<Get> get{};
    static constexpr Cmd<GetRng> getRng{};
    static constexpr Cmd<Remove> remove{};
    static constexpr Cmd<Splice> splice{};  
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
