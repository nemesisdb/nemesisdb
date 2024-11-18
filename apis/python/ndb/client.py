from abc import ABC, abstractmethod
from enum import Enum
from ndb.connection import Connection
from typing import Tuple
from asyncio import CancelledError



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


"""Used by KvClient and SessionClient to query the database.

Provides a function per database command.

Some functions have an optional 'tkn' parameter - this is only relevant when
using the SessionClient. When sessions are disabled, leave the 'tkn' as default.

Note: KV_SETQ and KV_ADDQ are not supported.
"""
class Client(ABC):
  
  def __init__(self):
    self.uri = ''
    self.listen_task = None
    self.api = Connection()

  
  async def open(self, uri: str):
    if uri == '':
      raise ValueError('URI empty')
    
    self.uri = uri
    self.listen_task = await self.api.start(self.uri)


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
  

  async def rmv(self, keys: tuple, tkn = 0) -> dict:
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


  async def contains(self, keys: tuple, tkn = 0) -> tuple:
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


  async def clear(self, tkn = 0) -> tuple:
    rsp = await self._send_query(KvCmd.CLEAR_REQ, {KvCmd.CLEAR_REQ : {}}, tkn)
    
    if self._is_rsp_valid(rsp, KvCmd.CLEAR_RSP):
      return (True, rsp[KvCmd.CLEAR_RSP]['cnt'])
    else:
      return (False, [])
    

  async def clear_set(self, keys: dict, tkn = 0) -> tuple:
    q = {KvCmd.CLEAR_SET_REQ : {'keys':keys}}
    rsp = await self._send_query(KvCmd.CLEAR_SET_REQ, q, tkn)    
    return (self._is_rsp_valid(rsp, KvCmd.CLEAR_SET_RSP), rsp[KvCmd.CLEAR_SET_RSP]['cnt'])


  async def save(self, name: str, tkn = 0):
    q = {KvCmd.SAVE_REQ : {'name':name}}
    rsp = await self._send_query(KvCmd.SAVE_REQ, q, tkn)
    return self._is_rsp_valid(rsp, KvCmd.SAVE_RSP, FieldValues.ST_SAVE_COMPLETE)

  
  async def load(self, name: str, tkn = 0):
    q = {KvCmd.LOAD_REQ : {'name':name}}
    rsp = await self._send_query(KvCmd.LOAD_REQ, q, tkn)
    if self._is_rsp_valid(rsp, KvCmd.LOAD_RSP, FieldValues.ST_LOAD_COMPLETE):
      return (True, rsp[KvCmd.LOAD_RSP]['keys'])
    else:
      return (False, 0)
  

  async def close(self):
    await self.api.close()


  def _is_rsp_valid(self, rsp: dict, cmd: str, expected = FieldValues.ST_SUCCESS) -> bool:
    return rsp[cmd][Fields.STATUS] == expected


  async def _doSetAdd(self, cmdName: str, rspName: str, keys: dict, tkn: int) -> bool:
    q = {cmdName : {'keys':keys}}   # cmdName is either KV_SET or KV_ADD
    rsp = await self._send_query(cmdName, q, tkn)
    return self._is_rsp_valid(rsp, rspName)
  
  
  async def _send_query(self, cmd: str, q: dict, tkn = 0):
    if tkn != 0:
      return await self._send_session_query(cmd, q, tkn)
    else:
      return await self._send_kv_query(q)


  async def _send_kv_query(self, q: dict):
    return await self.api.query(q)

  
  async def _send_session_query(self, cmd: str, q: dict, tkn: int):
    q[cmd]['tkn'] = tkn
    return await self.api.query(q)
