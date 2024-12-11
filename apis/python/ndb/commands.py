

class StValues:
  """
  There are more but we only require:
    ST_SUCCESS - command success
    ST_SAVE_COMPLETE - SH_SAVE or KV_SAVE success, data persisted
    ST_SAVE_ERROR - SH_SAVE or KV_SAVE fail
    ST_LOAD_COMPLETE - SH_LOAD or KV_LOAD success, data available
  """
  ST_SUCCESS = 1
  ST_SAVE_COMPLETE = 121
  ST_SAVE_ERROR = 123
  ST_LOAD_COMPLETE = 141


class Fields:
  STATUS    = 'st'
  TOKEN     = 'tkn'


class SvCmd:
  INFO_REQ    = 'SV_INFO'
  INFO_RSP    = 'SV_INFO_RSP'


class KvCmd:
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


class ShCmd:
  NEW_REQ       = 'SH_NEW'
  NEW_RSP       = 'SH_NEW_RSP'
  END_REQ       = 'SH_END'
  END_RSP       = 'SH_END_RSP'
  END_ALL_REQ   = 'SH_END_ALL'
  END_ALL_RSP   = 'SH_END_ALL_RSP'
  EXISTS_REQ    = 'SH_EXISTS'
  EXISTS_RSP    = 'SH_EXISTS_RSP'
  INFO_REQ      = 'SH_INFO'
  INFO_RSP      = 'SH_INFO_RSP'
  INFO_ALL_REQ  = 'SH_INFO_ALL'
  INFO_ALL_RSP  = 'SH_INFO_ALL_RSP'
  SAVE_REQ      = 'SH_SAVE'
  SAVE_RSP      = 'SH_SAVE_RSP'
  LOAD_REQ      = 'SH_LOAD'
  LOAD_RSP      = 'SH_LOAD_RSP'
  #
  SET_REQ       = 'SH_SET'
  SET_RSP       = 'SH_SET_RSP'
  ADD_REQ       = 'SH_ADD'
  ADD_RSP       = 'SH_ADD_RSP'
  GET_REQ       = 'SH_GET'
  GET_RSP       = 'SH_GET_RSP'
  RMV_REQ       = 'SH_RMV'
  RMV_RSP       = 'SH_RMV_RSP'
  COUNT_REQ     = 'SH_COUNT'
  COUNT_RSP     = 'SH_COUNT_RSP'
  CONTAINS_REQ  = 'SH_CONTAINS'
  CONTAINS_RSP  = 'SH_CONTAINS_RSP'
  CLEAR_REQ     = 'SH_CLEAR'
  CLEAR_RSP     = 'SH_CLEAR_RSP'
  CLEAR_SET_REQ = 'SH_CLEAR_SET'
  CLEAR_SET_RSP = 'SH_CLEAR_SET_RSP'
  KEYS_REQ      = 'SH_KEYS'
  KEYS_RSP      = 'SH_KEYS_RSP'


# TODO see if this can be tidied/reduced

class OArrCmd:
  CREATE_REQ        =    "OARR_CREATE"
  CREATE_RSP        =    "OARR_CREATE_RSP"
  DELETE_REQ        =    "OARR_DELETE"
  DELETE_RSP        =    "OARR_DELETE_RSP"
  DELETE_ALL_REQ    =    "OARR_DELETE_ALL"
  DELETE_ALL_RSP    =    "OARR_DELETE_ALL_RSP"
  SET_REQ           =    "OARR_SET"
  SET_RSP           =    "OARR_SET_RSP"
  SET_RNG_REQ       =    "OARR_SET_RNG"
  SET_RNG_RSP       =    "OARR_SET_RNG_RSP"
  GET_REQ           =    "OARR_GET"
  GET_RSP           =    "OARR_GET_RSP"
  GET_RNG_REQ       =    "OARR_GET_RNG"
  GET_RNG_RSP       =    "OARR_GET_RNG_RSP"
  LEN_REQ           =    "OARR_LEN"
  LEN_RSP           =    "OARR_LEN_RSP"
  SWAP_REQ          =    "OARR_SWAP"
  SWAP_RSP          =    "OARR_SWAP_RSP"
  EXIST_REQ         =    "OARR_EXIST"
  EXIST_RSP         =    "OARR_EXIST_RSP"
  CLEAR_REQ         =    "OARR_CLEAR"
  CLEAR_RSP         =    "OARR_CLEAR_RSP"
  

class IArrCmd:
  CREATE_REQ        =    "IARR_CREATE"
  CREATE_RSP        =    "IARR_CREATE_RSP"
  DELETE_REQ        =    "IARR_DELETE"
  DELETE_RSP        =    "IARR_DELETE_RSP"
  DELETE_ALL_REQ    =    "IARR_DELETE_ALL"
  DELETE_ALL_RSP    =    "IARR_DELETE_ALL_RSP"
  SET_REQ           =    "IARR_SET"
  SET_RSP           =    "IARR_SET_RSP"
  SET_RNG_REQ       =    "IARR_SET_RNG"
  SET_RNG_RSP       =    "IARR_SET_RNG_RSP"
  GET_REQ           =    "IARR_GET"
  GET_RSP           =    "IARR_GET_RSP"
  GET_RNG_REQ       =    "IARR_GET_RNG"
  GET_RNG_RSP       =    "IARR_GET_RNG_RSP"
  LEN_REQ           =    "IARR_LEN"
  LEN_RSP           =    "IARR_LEN_RSP"
  SWAP_REQ          =    "IARR_SWAP"
  SWAP_RSP          =    "IARR_SWAP_RSP"
  EXIST_REQ         =    "IARR_EXIST"
  EXIST_RSP         =    "IARR_EXIST_RSP"
  CLEAR_REQ         =    "IARR_CLEAR"
  CLEAR_RSP         =    "IARR_CLEAR_RSP"


class SortedIArrCmd:
  CREATE_REQ        =    "SIARR_CREATE"
  CREATE_RSP        =    "SIARR_CREATE_RSP"
  DELETE_REQ        =    "SIARR_DELETE"
  DELETE_RSP        =    "SIARR_DELETE_RSP"
  DELETE_ALL_REQ    =    "SIARR_DELETE_ALL"
  DELETE_ALL_RSP    =    "SIARR_DELETE_ALL_RSP"
  SET_REQ           =    "SIARR_SET"
  SET_RSP           =    "SIARR_SET_RSP"
  SET_RNG_REQ       =    "SIARR_SET_RNG"
  SET_RNG_RSP       =    "SIARR_SET_RNG_RSP"
  GET_REQ           =    "SIARR_GET"
  GET_RSP           =    "SIARR_GET_RSP"
  GET_RNG_REQ       =    "SIARR_GET_RNG"
  GET_RNG_RSP       =    "SIARR_GET_RNG_RSP"
  LEN_REQ           =    "SIARR_LEN"
  LEN_RSP           =    "SIARR_LEN_RSP"
  EXIST_REQ         =    "SIARR_EXIST"
  EXIST_RSP         =    "SIARR_EXIST_RSP"
  CLEAR_REQ         =    "SIARR_CLEAR"
  CLEAR_RSP         =    "SIARR_CLEAR_RSP"
  INTERSECT_REQ     =    "SIARR_INTERSECT"
  INTERSECT_RSP     =    "SIARR_INTERSECT_RSP"