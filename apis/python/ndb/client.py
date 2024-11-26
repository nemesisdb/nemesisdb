from abc import ABC
from enum import Enum
from ndb.connection import Connection
from ndb.logging import logger
from typing import Tuple, List
from asyncio import CancelledError
import logging



class SaveState(Enum):
  STARTED = 0
  COMPLETE = 1
  FAIL = 2



class FieldValues:
  ST_SUCCESS = 1
  ST_SAVE_START = 120
  ST_SAVE_COMPLETE = 121
  ST_SAVE_ERROR = 123
  ST_LOAD_COMPLETE = 141


class Fields:
  STATUS    = 'st'


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


class SvCmd:
  INFO_REQ    = 'SV_INFO'
  INFO_RSP    = 'SV_INFO_RSP'


class Client(ABC):
  """Used by KvClient and SessionClient to query the database.

  Provides a function per database command.

  Most functions have an optional 'tkn' parameter - this is only relevant when
  using the SessionClient. When sessions are disabled, leave the 'tkn' as default.

  Note: KV_SETQ and KV_ADDQ are not supported.
  """

  def __init__(self, debug = False):
    self.uri = ''
    self.listen_task = None
    self.api = Connection()
    if debug:
      logger.setLevel(logging.DEBUG)
      logger.addHandler(logging.StreamHandler())
      
  
  async def open(self, uri: str) -> bool:
    if uri == '':
      raise ValueError('URI empty')
    
    
    connected = await self.api.start(uri)
    logger.debug('Client: connected' if connected else 'Client: failed to connect')
    return connected


  async def set(self, keys: dict, tkn = 0) -> bool:
    return await self._doSetAdd(KvCmd.SET_REQ, KvCmd.SET_RSP, keys, tkn)
  

  async def add(self, keys: dict, tkn = 0) -> bool:
    return await self._doSetAdd(KvCmd.ADD_REQ, KvCmd.ADD_RSP, keys, tkn)


  async def get(self, keys: tuple, tkn = 0) -> Tuple[bool, dict]:
    q = {KvCmd.GET_REQ : {'keys':keys}}
    rsp = await self._send_query(KvCmd.GET_REQ, q, tkn)
    if self._is_rsp_valid(rsp, KvCmd.GET_RSP):
      return (True, rsp[KvCmd.GET_RSP]['keys'])
    else:
      return (False, dict())
  

  async def rmv(self, keys: tuple, tkn = 0) -> bool:
    q = {KvCmd.RMV_REQ : {'keys':keys}}
    rsp = await self._send_query(KvCmd.RMV_REQ, q, tkn)
    return self._is_rsp_valid(rsp, KvCmd.RMV_RSP)


  async def count(self, tkn = 0) -> tuple:
    q = {KvCmd.COUNT_REQ : {}}
    rsp = await self._send_query(KvCmd.COUNT_REQ, q, tkn)

    if self._is_rsp_valid(rsp, KvCmd.COUNT_RSP):
      return (True, rsp[KvCmd.COUNT_RSP]['cnt'])
    else:
      return (False, 0)


  async def contains(self, keys: tuple, tkn = 0) -> Tuple[bool, List]:
    q = {KvCmd.CONTAINS_REQ : {'keys':keys}}
    rsp = await self._send_query(KvCmd.CONTAINS_REQ, q, tkn)

    if self._is_rsp_valid(rsp, KvCmd.CONTAINS_RSP):
      return (True, rsp[KvCmd.CONTAINS_RSP]['contains'])
    else:
      (False, [])

  
  async def keys(self, tkn = 0) -> tuple:
    rsp = await self._send_query(KvCmd.KEYS_REQ, {KvCmd.KEYS_REQ : {}}, tkn)
    
    if self._is_rsp_valid(rsp, KvCmd.KEYS_RSP):
      return (True, rsp[KvCmd.KEYS_RSP]['keys'])
    else:
      return (False, [])


  async def clear(self, tkn = 0) -> Tuple[bool, int]:
    rsp = await self._send_query(KvCmd.CLEAR_REQ, {KvCmd.CLEAR_REQ : {}}, tkn)
    
    if self._is_rsp_valid(rsp, KvCmd.CLEAR_RSP):
      return (True, rsp[KvCmd.CLEAR_RSP]['cnt'])
    else:
      return (False, 0)
    

  async def clear_set(self, keys: dict, tkn = 0) -> Tuple[bool, int]:
    q = {KvCmd.CLEAR_SET_REQ : {'keys':keys}}
    rsp = await self._send_query(KvCmd.CLEAR_SET_REQ, q, tkn)    
    if self._is_rsp_valid(rsp, KvCmd.CLEAR_SET_RSP):
      return (True, rsp[KvCmd.CLEAR_SET_RSP]['cnt'])
    else:
      return (False, 0)


  async def save(self, name: str) -> bool:
    q = {KvCmd.SAVE_REQ : {'name':name}}
    rsp = await self._send_query(KvCmd.SAVE_REQ, q)
    return self._is_rsp_valid(rsp, KvCmd.SAVE_RSP, FieldValues.ST_SAVE_COMPLETE)

  
  async def load(self, name: str) -> Tuple[bool, int]:
    q = {KvCmd.LOAD_REQ : {'name':name}}
    rsp = await self._send_query(KvCmd.LOAD_REQ, q)
    if self._is_rsp_valid(rsp, KvCmd.LOAD_RSP, FieldValues.ST_LOAD_COMPLETE):
      return (True, rsp[KvCmd.LOAD_RSP]['keys'])
    else:
      return (False, 0)
  

  async def server_info(self) -> Tuple[bool, dict]:
    rsp = await self._send_query(SvCmd.INFO_REQ, {SvCmd.INFO_REQ : {}})
    if self._is_rsp_valid(rsp, SvCmd.INFO_RSP):
      info = dict(rsp.get(SvCmd.INFO_RSP))
      info.pop('st')
      return (True, info)
    else:
      return (False, dict())


  async def close(self):
    await self.api.close()


  def _is_rsp_valid(self, rsp: dict, cmd: str, expected = FieldValues.ST_SUCCESS) -> bool:
    valid = rsp[cmd][Fields.STATUS] == expected
    logger.debug(cmd + ' ' + ('Response Ok' if valid else f'Response Fail: {str(rsp)}'))
    return valid


  async def _doSetAdd(self, cmdName: str, rspName: str, keys: dict, tkn: int) -> bool:
    q = {cmdName : {'keys':keys}}   # cmdName is either KV_SET or KV_ADD
    rsp = await self._send_query(cmdName, q, tkn)
    return self._is_rsp_valid(rsp, rspName)
  
  
  async def _send_query(self, cmd: str, q: dict, tkn = 0):
    logger.debug('Send query: '+ str(q))
    if tkn != 0:
      return await self._send_session_query(cmd, q, tkn)
    else:
      return await self._send_kv_query(q)


  async def _send_kv_query(self, q: dict) -> dict:
    return await self.api.query(q)

  
  async def _send_session_query(self, cmd: str, q: dict, tkn: int) -> dict:
    q[cmd]['tkn'] = tkn
    return await self.api.query(q)
