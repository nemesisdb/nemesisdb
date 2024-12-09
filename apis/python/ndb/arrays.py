from ndb.commands import StValues, ArrCmd
from ndb.client import NdbClient
from ndb.common import raise_if, raise_if_empty
from typing import List

#region Object Arrays
class OArrays:
  
  def __init__(self, client: NdbClient):
    self.client = client


  async def oarr_create(self, name: str, len: int) -> int:
    raise_if_empty(name)
    raise_if(len, 'must be > 0', lambda v: v <= 0)
    await self.client.sendCmd(ArrCmd.CREATE_REQ, ArrCmd.CREATE_RSP, {'name':name, 'len':len})


  async def oarr_delete(self, name: str) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(ArrCmd.DELETE_REQ, ArrCmd.DELETE_RSP, {'name':name})

  
  async def oarr_delete_all(self) -> None:
    await self.client.sendCmd(ArrCmd.DELETE_ALL_REQ, ArrCmd.DELETE_ALL_RSP, {})


  async def oarr_set(self, name: str, pos: int, item: dict) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(ArrCmd.SET_REQ, ArrCmd.SET_RSP, {'name':name, 'pos': pos, 'item':item})


  async def oarr_set_rng(self, name: str, pos: int, items: List[dict]) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(ArrCmd.SET_RNG_REQ, ArrCmd.SET_RNG_RSP, {'name':name, 'pos': pos, 'items':items})


  async def oarr_get(self, name: str, pos: int) -> dict:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(ArrCmd.GET_REQ, ArrCmd.GET_RSP, {'name':name, 'pos':pos})
    return rsp[ArrCmd.GET_RSP]['item']


  async def oarr_get_rng(self, name: str, start: int, stop = None) -> List[dict]:
    raise_if_empty(name)

    if stop == None:
      rng = [start]
    elif start > stop:
      raise ValueError('start > stop')
    else:
      rng = [start, stop]
        
    rsp = await self.client.sendCmd(ArrCmd.GET_RNG_REQ, ArrCmd.GET_RNG_RSP, {'name':name, 'rng':rng})
    return rsp[ArrCmd.GET_RNG_RSP]['items']
  
  
  async def oarr_len(self, name: str) -> int:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(ArrCmd.LEN_REQ, ArrCmd.LEN_RSP, {'name':name})
    return rsp[ArrCmd.LEN_RSP]['len']


  async def oarr_swap(self, name: str, posA: int, posB: int) -> int:
    raise_if_empty(name)
    await self.client.sendCmd(ArrCmd.SWAP_REQ, ArrCmd.SWAP_RSP, {'name':name, 'posA':posA, 'posB':posB})

  
  async def oarr_exist(self, name: str) -> bool:
    # this query uses the status to indicate result: StValue.ST_SUCCESS means the array exists
    raise_if_empty(name)
    # checkStatus=False because 'status' not being success is not an error
    rsp = await self.client.sendCmd(ArrCmd.EXIST_REQ, ArrCmd.EXIST_RSP, {'name':name}, checkStatus=False)    
    return rsp[ArrCmd.EXIST_RSP]['st'] == StValues.ST_SUCCESS
  

  async def oarr_clear(self, name: str, start: int, stop: int) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(ArrCmd.CLEAR_REQ, ArrCmd.CLEAR_RSP, {'name':name, 'rng':[start, stop]})

#endregion