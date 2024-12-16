from ndb.commands import (StValues, Fields, ObjListCmds)
from ndb.client import NdbClient
from ndb.common import raise_if, raise_if_empty, raise_if_equal, raise_if_lt, raise_if_not
from typing import List
from abc import ABC, abstractmethod


class _Lists(ABC):

  def __init__(self, client: NdbClient):
    self.client = client
    self.cmds = self.getCommandNames()  # calls child class


  @abstractmethod
  def getCommandNames(self):
    return
  
  
  async def create(self, name: str) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.CREATE_REQ, self.cmds.CREATE_RSP, {'name':name})


  async def delete_all(self) -> None:
    await self.client.sendCmd(self.cmds.DELETE_ALL_REQ, self.cmds.DELETE_ALL_RSP, {})

  
  async def delete(self, name: str) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.DELETE_REQ, self.cmds.DELETE_RSP, {'name':name})

  
  async def exist(self, name: str) -> None:
    raise_if_empty(name)
    # don't check status: EXIST response has 'st' success if list exists or NotExist otherwise (so not an error)
    rsp = await self.client.sendCmd(self.cmds.EXIST_REQ, self.cmds.EXIST_RSP, {'name':name}, checkStatus=False)
    return rsp[self.cmds.EXIST_RSP][Fields.STATUS] == StValues.ST_SUCCESS


class ObjLists(_Lists):
  def __init__(self, client):
    super().__init__(client)


  # override of base class 
  def getCommandNames(self) -> ObjListCmds:
    return ObjListCmds()
  
  
  # NOTE: server contains 'pos' and 'size' in the response. Only return pos here.
  async def add(self, name: str, items: List[dict], pos = None) -> int:
    raise_if_empty(name)
    if pos is None:
      args = {'name':name, 'items':items}      
    else:
      raise_if_not(isinstance(pos, int), 'pos must be int', TypeError)
      raise_if_lt(pos, 0, 'pos < 0')    
      args = {'name':name, 'items':items, 'pos':pos}

    rsp = await self.client.sendCmd(self.cmds.ADD_REQ, self.cmds.ADD_RSP, args)
    return rsp[self.cmds.ADD_RSP]['pos']


  async def add_head(self, name: str, items: List[dict]) -> None:
    await self.add(name, items, pos=0)


  async def add_tail(self, name: str, items: List[dict]) -> None:
    return await self.add(name, items, pos=None)


  async def set_rng(self, name: str, items: List[dict], start: int) -> None:
    raise_if_empty(name)
    raise_if_lt(start, 0, 'start < 0') 
    await self.client.sendCmd(self.cmds.SET_RNG_REQ, self.cmds.SET_RNG_RSP, {'name':name, 'items':items, 'pos':start})


  async def get(self, name: str, pos: int) -> dict:
    raise_if_empty(name)
    if pos is None:
      args = {'name':name}      
    else:
      raise_if_lt(pos, 0, 'pos < 0')
      args = {'name':name, 'pos':pos}

    rsp = await self.client.sendCmd(self.cmds.GET_REQ, self.cmds.GET_RSP, args)
    return rsp[self.cmds.GET_RSP]['item']
  

  async def get_head(self, name: str) -> dict:
    return await self.get(name, pos=0)
  
  
  async def get_tail(self, name: str) -> dict:
    return await self.get(name, pos=None)
  

  async def get_rng(self, name: str, start: int, stop=None) -> List[dict]:
    raise_if_empty(name)
    raise_if_lt(start, 0, 'start < 0') 
    if stop is None:
      rng = [start]
    else:
      raise_if_not(isinstance(stop, int), 'stop must be int', TypeError)
      raise_if_lt(stop, 0, 'stop < 0')
      rng = [start, stop]
      
    rsp = await self.client.sendCmd(self.cmds.GET_RNG_REQ, self.cmds.GET_RNG_RSP, {'name':name, 'rng':rng})
    return rsp[self.cmds.GET_RNG_RSP]['items']