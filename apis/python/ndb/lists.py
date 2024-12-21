from ndb.commands import (StValues, Fields, ObjListCmds)
from ndb.client import NdbClient
from ndb.common import raise_if_empty, raise_if_lt, raise_if_not
from typing import List
from abc import ABC, abstractmethod


class _Lists(ABC):
  """ Functions common to all lists """
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

  
  async def exist(self, name: str) -> bool:
    raise_if_empty(name)
    # don't check status: EXIST response has 'st' success if list exists or NotExist otherwise (so not an error)
    rsp = await self.client.sendCmd(self.cmds.EXIST_REQ, self.cmds.EXIST_RSP, {'name':name}, checkStatus=False)
    return rsp[self.cmds.EXIST_RSP][Fields.STATUS] == StValues.ST_SUCCESS
  

  async def length(self, name: str) -> int:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(self.cmds.LEN_REQ, self.cmds.LEN_RSP, {'name':name})
    return rsp[self.cmds.LEN_RSP]['len']


  async def remove(self, name: str, start: int, stop = None) -> int:
    raise_if_empty(name)
    raise_if_lt(start, 0, 'start < 0')
    if stop is None:
      args = {'name':name, 'rng':[start]}
    else:
      raise_if_lt(stop, 0, 'stop < 0')
      raise_if_lt(stop, start, 'stop < start')
      args = {'name':name, 'rng':[start, stop]}

    rsp = await self.client.sendCmd(self.cmds.RMV_REQ, self.cmds.RMV_RSP, args)
    return rsp[self.cmds.RMV_RSP]['size']


  async def remove_head(self, name: str) -> int:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(self.cmds.RMV_REQ, self.cmds.RMV_RSP, {'name':name, 'head':True})
    return rsp[self.cmds.RMV_RSP]['size']
  

  async def remove_tail(self, name: str) -> int:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(self.cmds.RMV_REQ, self.cmds.RMV_RSP, {'name':name, 'tail':True})
    return rsp[self.cmds.RMV_RSP]['size']
  

  async def splice(self, destName: str, srcName: str, srcStart: int, srcEnd = None, destPos = None) -> None:
    raise_if_empty(destName)
    raise_if_empty(srcName)
    raise_if_lt(srcStart, 0, 'srcEnd < 0')

    args = {'srcName':srcName, 'destName':destName}
    
    if srcEnd is not None:
      raise_if_not(isinstance(srcEnd, int), 'srcEnd must be int', TypeError)
      raise_if_lt(srcEnd, 0, 'srcEnd < 0')
      raise_if_lt(srcEnd, srcStart, 'srcEnd < srcStart')
      args['srcRng'] = [srcStart, srcEnd]      
    else:
      args['srcRng'] = [srcStart]

    if destPos is not None:
      raise_if_not(isinstance(destPos, int), 'destPos must be int', TypeError)
      raise_if_lt(destPos, 0, 'destPos < 0')
      args['destPos'] = destPos

    await self.client.sendCmd(self.cmds.SPLICE_REQ, self.cmds.SPLICE_RSP, args)
   


class ObjLists(_Lists):
  "Object lists"
  def __init__(self, client):
    super().__init__(client)


  # override of base class 
  def getCommandNames(self) -> ObjListCmds:
    return ObjListCmds()
  
  
  # NOTE: server returns 'pos' and 'size', but add() only returns pos.
  async def add(self, name: str, items: list[dict] | dict, pos = None) -> int:
    raise_if_empty(name)

    itemsValid = items is not None and isinstance(items, (dict, list))

    raise_if_not(itemsValid, 'items must be List[dict] or dict')
    raise_if_not(pos is None or isinstance(pos, int), 'pos must be int or None', TypeError)

    data = []

    if isinstance(items, dict):
      data.append(items)
    else:
      data = items

    if pos is None:
      args = {'name':name, 'items':data}      
    else:
      raise_if_lt(pos, 0, 'pos < 0')    
      args = {'name':name, 'items':data, 'pos':pos}

    rsp = await self.client.sendCmd(self.cmds.ADD_REQ, self.cmds.ADD_RSP, args)
    return rsp[self.cmds.ADD_RSP]['pos']
  

  async def add_head(self, name: str, items=list[dict]|dict) -> None:
    await self.add(name, items, pos=0)


  async def add_tail(self, name: str, items=list[dict]|dict) -> int:
    return await self.add(name, items, pos=None)
      

  async def set(self, name: str, items=list[dict]|dict, start=0) -> None:
    raise_if_empty(name)
    raise_if_lt(start, 0, 'start < 0') 

    itemsValid = items is not None and isinstance(items, (dict, list))

    raise_if_not(itemsValid, 'items must be List[dict] or dict')
    raise_if_not(start is None or isinstance(start, int), 'start must be int', TypeError)

    args = {'name':name, 'pos':start}

    if isinstance(items, dict):
      args['items'] = [items]
    else:
      args['items'] = items
      
    await self.client.sendCmd(self.cmds.SET_RNG_REQ, self.cmds.SET_RNG_RSP, args)


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
      raise_if_lt(stop, start, 'stop < start')
      rng = [start, stop]
      
    rsp = await self.client.sendCmd(self.cmds.GET_RNG_REQ, self.cmds.GET_RNG_RSP, {'name':name, 'rng':rng})
    return rsp[self.cmds.GET_RNG_RSP]['items']
  