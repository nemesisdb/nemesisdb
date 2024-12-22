from ndb.commands import (StValues, KvCmds)
from ndb.client import NdbClient
from ndb.common import raise_if_empty, raise_if_not
from typing import List, Any


class KV:
  "Key Value"


  def __init__(self, client: NdbClient):
    self.client = client
    self.cmds = KvCmds()


  async def set(self, keys: dict) -> None:
    await self.client.sendCmd(self.cmds.SET_REQ, self.cmds.SET_RSP, {'keys':keys})
  

  async def add(self, keys: dict) -> None:
    await self.client.sendCmd(self.cmds.ADD_REQ, self.cmds.ADD_RSP, {'keys':keys})


  async def get(self, keys = None, key=None) -> dict | Any:
    if key is not None and keys is not None:
      raise ValueError('Both keys and key are set')
    elif key != None:
      raise_if_not(isinstance(key, str), 'key must be a string')
      return await self._kv_get_single(key)
    else:
      raise_if_not(isinstance(keys, tuple), 'keys must be a tuple')
      return await self._kv_get_multiple(keys)

  
  async def _kv_get_single(self, key: str):
    rsp = await self.client.sendCmd(self.cmds.GET_REQ, self.cmds.GET_RSP, {'keys':[key]})
    if key in rsp[self.cmds.GET_RSP]['keys']:
      return rsp[self.cmds.GET_RSP]['keys'][key]
    else:
      return None
  

  async def _kv_get_multiple(self, keys: tuple) -> dict:
    rsp = await self.client.sendCmd(self.cmds.GET_REQ, self.cmds.GET_RSP, {'keys':keys})
    return rsp[self.cmds.GET_RSP]['keys']
  

  async def rmv(self, keys: tuple) -> None:
    if len(keys) == 0:
      raise ValueError('Keys empty')
    await self.client.sendCmd(self.cmds.RMV_REQ, self.cmds.RMV_RSP, {'keys':keys})


  async def count(self) -> int:
    rsp = await self.client.sendCmd(self.cmds.COUNT_REQ, self.cmds.COUNT_RSP, {})
    return rsp[self.cmds.COUNT_RSP]['cnt']


  async def contains(self, keys: tuple) -> List[str]:
    rsp = await self.client.sendCmd(self.cmds.CONTAINS_REQ, self.cmds.CONTAINS_RSP, {'keys':keys})
    return rsp[self.cmds.CONTAINS_RSP]['contains']

  
  async def keys(self) -> List[str]:
    rsp = await self.client.sendCmd(self.cmds.KEYS_REQ, self.cmds.KEYS_RSP, {})
    return rsp[self.cmds.KEYS_RSP]['keys']
  

  async def clear(self) -> int:
    rsp = await self.client.sendCmd(self.cmds.CLEAR_REQ, self.cmds.CLEAR_RSP, {})
    return rsp[self.cmds.CLEAR_RSP]['cnt']
        

  async def clear_set(self, keys: dict) -> int:
    rsp = await self.client.sendCmd(self.cmds.CLEAR_SET_REQ, self.cmds.CLEAR_SET_RSP, {'keys':keys})
    return rsp[self.cmds.CLEAR_SET_RSP]['cnt'] 


  async def save(self, name: str) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.SAVE_REQ, self.cmds.SAVE_RSP, {'name':name}, StValues.ST_SAVE_COMPLETE)
    

  async def load(self, name: str) -> int:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(self.cmds.LOAD_REQ, self.cmds.LOAD_RSP, {'name':name}, StValues.ST_LOAD_COMPLETE)
    return rsp[self.cmds.LOAD_RSP]['keys']
