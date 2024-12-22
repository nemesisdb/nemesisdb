class StValues:
  """
  There are more but we only require:

    ST_SUCCESS        - Command success
    ST_SAVE_COMPLETE  - KV_SAVE success, data persisted
    ST_SAVE_ERROR     - KV_SAVE fail
    ST_LOAD_COMPLETE  - KV_LOAD success, data available
  """
  ST_SUCCESS = 1
  ST_SAVE_COMPLETE = 120
  ST_SAVE_ERROR = 123
  ST_LOAD_COMPLETE = 141


class Fields:
  STATUS    = 'st'


class SvCmds:
  INFO_REQ    = 'SV_INFO'
  INFO_RSP    = 'SV_INFO_RSP'


class KvCmds:
  SET_REQ       = 'KV_SET'
  SET_RSP       = 'KV_SET_RSP'
  ADD_REQ       = 'KV_ADD'
  ADD_RSP       = 'KV_ADD_RSP'
  GET_REQ       = 'KV_GET'
  GET_RSP       = 'KV_GET_RSP'
  RMV_REQ       = 'KV_RMV'
  RMV_RSP       = 'KV_RMV_RSP'
  COUNT_REQ     = 'KV_COUNT'
  COUNT_RSP     = 'KV_COUNT_RSP'
  CONTAINS_REQ  = 'KV_CONTAINS'
  CONTAINS_RSP  = 'KV_CONTAINS_RSP'
  CLEAR_REQ     = 'KV_CLEAR'
  CLEAR_RSP     = 'KV_CLEAR_RSP'
  CLEAR_SET_REQ = 'KV_CLEAR_SET'
  CLEAR_SET_RSP = 'KV_CLEAR_SET_RSP'
  KEYS_REQ      = 'KV_KEYS'
  KEYS_RSP      = 'KV_KEYS_RSP'
  SAVE_REQ      = "KV_SAVE"
  SAVE_RSP      = "KV_SAVE_RSP"
  LOAD_REQ      = "KV_LOAD"
  LOAD_RSP      = "KV_LOAD_RSP"


#region Arrays

class CommonArrCmds:
  def __init__(self, ident: str):
    self.CREATE_REQ, self.CREATE_RSP          = self.make(ident, "CREATE")
    self.DELETE_REQ, self.DELETE_RSP          = self.make(ident, "DELETE")
    self.DELETE_ALL_REQ, self.DELETE_ALL_RSP  = self.make(ident, "DELETE_ALL")
    self.SET_REQ, self.SET_RSP                = self.make(ident, "SET")
    self.SET_RNG_REQ, self.SET_RNG_RSP        = self.make(ident, "SET_RNG")
    self.GET_REQ, self.GET_RSP                = self.make(ident, "GET")
    self.GET_RNG_REQ, self.GET_RNG_RSP        = self.make(ident, "GET_RNG")
    self.LEN_REQ, self.LEN_RSP                = self.make(ident, "LEN")
    self.USED_REQ, self.USED_RSP              = self.make(ident, "USED")    
    self.EXIST_REQ, self.EXIST_RSP            = self.make(ident, "EXIST")
    self.CLEAR_REQ, self.CLEAR_RSP            = self.make(ident, "CLEAR")


  def make(self, ident: str, cmd: str):
    req = ident+'_'+cmd
    return (req, req+'_RSP')


class UnsortedArrCmds(CommonArrCmds):
  def __init__(self, ident):
    super().__init__(ident)
    self.SWAP_REQ, self.SWAP_RSP = self.make(ident, "SWAP")


class SortedArrCmds(CommonArrCmds):
  def __init__(self, ident):
    super().__init__(ident)
    self.INTERSECT_REQ, self.INTERSECT_RSP = self.make(ident, "INTERSECT")
    self.MIN_REQ, self.MIN_RSP = self.make(ident, "MIN")
    self.MAX_REQ, self.MAX_RSP = self.make(ident, "MAX")



class OArrCmd(UnsortedArrCmds):
  def __init__(self):
    super().__init__('OARR')
    

class IArrCmd(UnsortedArrCmds):
  def __init__(self):
    super().__init__('IARR')
  

class StringArrCmd(UnsortedArrCmds):
  def __init__(self):
    super().__init__('STRARR')
  

class SortedIArrCmd(SortedArrCmds):
  def __init__(self):
    super().__init__('SIARR')


class SortedStrArrCmd(SortedArrCmds):
  def __init__(self):
    super().__init__('SSTRARR')

#endregion


#region Lists
class CommonListCmds:
  def __init__(self, ident: str):
    self.CREATE_REQ, self.CREATE_RSP          = self.make(ident, "CREATE")
    self.DELETE_REQ, self.DELETE_RSP          = self.make(ident, "DELETE")
    self.DELETE_ALL_REQ, self.DELETE_ALL_RSP  = self.make(ident, "DELETE_ALL")
    self.ADD_REQ, self.ADD_RSP                = self.make(ident, "ADD")
    self.SET_RNG_REQ, self.SET_RNG_RSP        = self.make(ident, "SET_RNG")
    self.GET_REQ, self.GET_RSP                = self.make(ident, "GET")
    self.GET_RNG_REQ, self.GET_RNG_RSP        = self.make(ident, "GET_RNG")
    self.LEN_REQ, self.LEN_RSP                = self.make(ident, "LEN")
    self.EXIST_REQ, self.EXIST_RSP            = self.make(ident, "EXIST")
    self.RMV_REQ, self.RMV_RSP                = self.make(ident, "RMV")
    self.SPLICE_REQ, self.SPLICE_RSP          = self.make(ident, "SPLICE")


  def make(self, ident: str, cmd: str):
    req = ident+'_'+cmd
    return (req, req+'_RSP')
  
  
class ObjListCmds(CommonListCmds):
  def __init__(self):
    super().__init__('OLST')



#endregion