

from ndb.client import Client
from ndb.client import StValues
from typing import Tuple, List


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


"""Client for when server has sessions disabled.
If sessions are enabled, use SessionClient.
"""
class KvClient(Client):
  # no extra work required, just use Client functions as they are.
  def __init__(self, debug = False):
    super().__init__(debug)


  async def set(self, keys: dict, tkn = 0) -> bool:
    return await self._doSetAdd(KvCmd.SET_REQ, KvCmd.SET_RSP, keys)
  

  async def add(self, keys: dict, tkn = 0) -> bool:
    return await self._doSetAdd(KvCmd.ADD_REQ, KvCmd.ADD_RSP, keys)


  async def get(self, keys: tuple, tkn = 0) -> Tuple[bool, dict]:
    ok, rsp = await self.sendCmd(KvCmd.GET_REQ, KvCmd.GET_RSP, {'keys':keys})
    return (ok, rsp[KvCmd.GET_RSP]['keys'] if ok else dict())


  async def rmv(self, keys: tuple, tkn = 0) -> bool:
    ok, _ = await self.sendCmd(KvCmd.RMV_REQ, KvCmd.RMV_RSP, {'keys':keys})
    return ok


  async def count(self, tkn = 0) -> tuple:
    ok, rsp = await self.sendCmd(KvCmd.COUNT_REQ, KvCmd.COUNT_RSP, {})
    return (ok, rsp[KvCmd.COUNT_RSP]['cnt'] if ok else 0)


  async def contains(self, keys: tuple, tkn = 0) -> Tuple[bool, List]:
    ok, rsp = await self.sendCmd(KvCmd.CONTAINS_REQ, KvCmd.CONTAINS_RSP, {'keys':keys})
    return (ok, rsp[KvCmd.CONTAINS_RSP]['contains'] if ok else [])

  
  async def keys(self, tkn = 0) -> tuple:
    ok, rsp = await self.sendCmd(KvCmd.KEYS_REQ, KvCmd.KEYS_RSP, {})
    return (ok, rsp[KvCmd.KEYS_RSP]['keys'] if ok else [])
  

  async def clear(self, tkn = 0) -> Tuple[bool, int]:
    ok, rsp = await self.sendCmd(KvCmd.CLEAR_REQ, KvCmd.CLEAR_RSP, {})
    return (ok, rsp[KvCmd.CLEAR_RSP]['cnt'] if ok else 0)
        

  async def clear_set(self, keys: dict, tkn = 0) -> Tuple[bool, int]:
    ok, rsp = await self.sendCmd(KvCmd.CLEAR_SET_REQ, KvCmd.CLEAR_SET_RSP, {'keys':keys})
    return (ok, rsp[KvCmd.CLEAR_SET_RSP]['cnt'] if ok else 0)


  async def save(self, name: str) -> bool:
    ok, _ = await self.sendCmd(KvCmd.SAVE_REQ, KvCmd.SAVE_RSP, {'name':name}, StValues.ST_SAVE_COMPLETE)
    return ok
    

  async def load(self, name: str) -> Tuple[bool, int]:
    ok, rsp = await self.sendCmd(KvCmd.LOAD_REQ, KvCmd.LOAD_RSP, {'name':name}, StValues.ST_LOAD_COMPLETE)
    return (ok, rsp[KvCmd.LOAD_RSP]['keys'] if ok else 0)


  async def _doSetAdd(self, cmdName: str, rspName: str, keys: dict) -> bool:
    return await self.sendCmd(cmdName, rspName, {'keys':keys})