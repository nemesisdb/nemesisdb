from ndb.commands import StValues, OArrCmd, IArrCmd, SortedIArrCmd
from ndb.client import NdbClient
from ndb.common import raise_if, raise_if_empty, raise_if_equal
from typing import List
from abc import ABC, abstractmethod


class _Arrays(ABC):

  def __init__(self, client: NdbClient):
    self.client = client
    self.cmds = self.getCommandNames()

  @abstractmethod
  def getCommandNames(self):
    return

  async def arr_create(self, name: str, len: int) -> int:
    raise_if_empty(name)
    raise_if(len, 'must be > 0', lambda v: v <= 0)
    await self.client.sendCmd(self.cmds.CREATE_REQ, self.cmds.CREATE_RSP, {'name':name, 'len':len})


  async def arr_delete(self, name: str) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.DELETE_REQ, self.cmds.DELETE_RSP, {'name':name})

  
  async def arr_delete_all(self) -> None:
    await self.client.sendCmd(self.cmds.DELETE_ALL_REQ, self.cmds.DELETE_ALL_RSP, {})

  
  async def arr_len(self, name: str) -> int:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(self.cmds.LEN_REQ, self.cmds.LEN_RSP, {'name':name})
    return rsp[self.cmds.LEN_RSP]['len']


  async def arr_swap(self, name: str, posA: int, posB: int) -> int:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.SWAP_REQ, self.cmds.SWAP_RSP, {'name':name, 'posA':posA, 'posB':posB})

  
  async def arr_exist(self, name: str) -> bool:
    # this query uses the status to indicate result: StValue.ST_SUCCESS means the array exists
    raise_if_empty(name)
    # checkStatus=False because 'status' not being success is not an error
    rsp = await self.client.sendCmd(self.cmds.EXIST_REQ, self.cmds.EXIST_RSP, {'name':name}, checkStatus=False)    
    return rsp[self.cmds.EXIST_RSP]['st'] == StValues.ST_SUCCESS
  

  async def arr_clear(self, name: str, start: int, stop: int) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.CLEAR_REQ, self.cmds.CLEAR_RSP, {'name':name, 'rng':[start, stop]})


#region OArrays
class OArrays(_Arrays):  
  def __init__(self, client: NdbClient):
    super().__init__(client)

  # override of base class 
  def getCommandNames(self) -> OArrCmd:
    return OArrCmd()

  async def arr_set(self, name: str, pos: int, item: dict) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.SET_REQ, self.cmds.SET_RSP, {'name':name, 'pos': pos, 'item':item})


  async def arr_set_rng(self, name: str, pos: int, items: List[dict]) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.SET_RNG_REQ, self.cmds.SET_RNG_RSP, {'name':name, 'pos': pos, 'items':items})


  async def arr_get(self, name: str, pos: int) -> dict:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(self.cmds.GET_REQ, self.cmds.GET_RSP, {'name':name, 'pos':pos})
    return rsp[self.cmds.GET_RSP]['item']


  async def arr_get_rng(self, name: str, start: int, stop = None) -> List[dict]:
    raise_if_empty(name)

    reqName = self.cmds.GET_RNG_REQ
    rspName = self.cmds.GET_RNG_RSP

    if stop == None:
      rng = [start]
    elif start > stop:
      raise ValueError('start > stop')
    else:
      rng = [start, stop]
        
    rsp = await self.client.sendCmd(reqName, rspName, {'name':name, 'rng':rng})
    return rsp[rspName]['items']

#endregion


#region IArray

class IntArrays(_Arrays):
  def __init__(self, client: NdbClient):
    super().__init__(client)

  # override of base class 
  def getCommandNames(self) -> IArrCmd:
    return IArrCmd()
  

  async def arr_set(self, name: str, pos: int, item: int) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.SET_REQ, self.cmds.SET_RSP, {'name':name, 'pos': pos, 'item':item})


  async def arr_set_rng(self, name: str, pos: int, items: List[int]) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.SET_RNG_REQ, self.cmds.SET_RNG_RSP, {'name':name, 'pos': pos, 'items':items})


  async def arr_get(self, name: str, pos: int) -> int:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(self.cmds.GET_REQ, self.cmds.GET_RSP, {'name':name, 'pos':pos})
    return rsp[self.cmds.GET_RSP]['item']


  async def arr_get_rng(self, name: str, start: int, stop = None) -> List[int]:
    raise_if_empty(name)

    reqName = self.cmds.GET_RNG_REQ
    rspName = self.cmds.GET_RNG_RSP

    if stop == None:
      rng = [start]
    elif start > stop:
      raise ValueError('start > stop')
    else:
      rng = [start, stop]
        
    rsp = await self.client.sendCmd(reqName, rspName, {'name':name, 'rng':rng})
    return rsp[rspName]['items']
  
#endregion


#region Sorted IArray

class SortedIntArrays(_Arrays):
  def __init__(self, client: NdbClient):
    super().__init__(client)

  # override of base class 
  def getCommandNames(self) -> SortedIArrCmd:
    return SortedIArrCmd()
  

  async def arr_set(self, name: str, item: int) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.SET_REQ, self.cmds.SET_RSP, {'name':name, 'item':item})


  async def arr_set_rng(self, name: str, items: List[int]) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(self.cmds.SET_RNG_REQ, self.cmds.SET_RNG_RSP, {'name':name, 'items':items})


  async def arr_get(self, name: str, pos: int) -> int:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(self.cmds.GET_REQ, self.cmds.GET_RSP, {'name':name, 'pos':pos})
    return rsp[self.cmds.GET_RSP]['item']


  async def arr_get_rng(self, name: str, start: int, stop = None) -> List[int]:
    raise_if_empty(name)

    reqName = self.cmds.GET_RNG_REQ
    rspName = self.cmds.GET_RNG_RSP

    if stop == None:
      rng = [start]
    elif start > stop:
      raise ValueError('start > stop')
    else:
      rng = [start, stop]
        
    rsp = await self.client.sendCmd(reqName, rspName, {'name':name, 'rng':rng})
    return rsp[rspName]['items']
  

  async def arr_intersect(self, arrA: str, arrB: str) -> List[int]:
    raise_if_empty(arrA)
    raise_if_empty(arrB)
    raise_if_equal(arrA, arrB, 'Intersect on the same arrays')
    rsp = await self.client.sendCmd(self.cmds.INTERSECT_REQ, self.cmds.INTERSECT_RSP, {'srcA':arrA, 'srcB':arrB})
    return rsp[self.cmds.INTERSECT_RSP]['items']
  

  async def arr_swap(self, name: str, posA: int, posB: int) -> int:
    raise NotImplementedError('Swap not permitted on sorted array')

#endregion