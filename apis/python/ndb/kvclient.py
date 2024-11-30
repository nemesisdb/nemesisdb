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


class KvClient(Client):
  """Client for storing independent keys (i.e. keys not in a session).
  """
  
  def __init__(self, debug = False):
    super().__init__(debug)


  async def set(self, keys: dict):
    await self._sendCmd(KvCmd.SET_REQ, KvCmd.SET_RSP, {'keys':keys})
  

  async def add(self, keys: dict) -> bool:
    await self._sendCmd(KvCmd.ADD_REQ, KvCmd.ADD_RSP, {'keys':keys})


  async def get(self, keys: tuple) -> dict:
    rsp = await self._sendCmd(KvCmd.GET_REQ, KvCmd.GET_RSP, {'keys':keys})
    return rsp[KvCmd.GET_RSP]['keys']


  async def rmv(self, keys: tuple):
    await self._sendCmd(KvCmd.RMV_REQ, KvCmd.RMV_RSP, {'keys':keys})


  async def count(self) -> int:
    rsp = await self._sendCmd(KvCmd.COUNT_REQ, KvCmd.COUNT_RSP, {})
    return rsp[KvCmd.COUNT_RSP]['cnt']


  async def contains(self, keys: tuple) -> List:
    rsp = await self._sendCmd(KvCmd.CONTAINS_REQ, KvCmd.CONTAINS_RSP, {'keys':keys})
    return rsp[KvCmd.CONTAINS_RSP]['contains']

  
  async def keys(self) -> int:
    rsp = await self._sendCmd(KvCmd.KEYS_REQ, KvCmd.KEYS_RSP, {})
    return rsp[KvCmd.KEYS_RSP]['keys']
  

  async def clear(self) -> int:
    rsp = await self._sendCmd(KvCmd.CLEAR_REQ, KvCmd.CLEAR_RSP, {})
    return rsp[KvCmd.CLEAR_RSP]['cnt']
        

  async def clear_set(self, keys: dict) -> int:
    rsp = await self._sendCmd(KvCmd.CLEAR_SET_REQ, KvCmd.CLEAR_SET_RSP, {'keys':keys})
    return rsp[KvCmd.CLEAR_SET_RSP]['cnt'] 


  async def save(self, name: str):
    await self._sendCmd(KvCmd.SAVE_REQ, KvCmd.SAVE_RSP, {'name':name}, StValues.ST_SAVE_COMPLETE)
    

  async def load(self, name: str) -> int:
    rsp = await self._sendCmd(KvCmd.LOAD_REQ, KvCmd.LOAD_RSP, {'name':name}, StValues.ST_LOAD_COMPLETE)
    return rsp[KvCmd.LOAD_RSP]['keys']
