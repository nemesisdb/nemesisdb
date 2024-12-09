from ndb.commands import StValues, OArrCmd, IArrCmd
from ndb.client import NdbClient
from ndb.common import raise_if, raise_if_empty
from typing import List


class _Arrays:
  def __init__(self, client: NdbClient):
    self.client = client

  async def arr_create(self, reqName:str, rspName:str, name: str, len: int) -> int:
    raise_if_empty(name)
    raise_if(len, 'must be > 0', lambda v: v <= 0)
    await self.client.sendCmd(reqName, rspName, {'name':name, 'len':len})


  async def arr_delete(self, reqName:str, rspName:str, name: str) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(reqName, rspName, {'name':name})

  
  async def arr_delete_all(self, reqName:str, rspName:str) -> None:
    await self.client.sendCmd(reqName, rspName, {})


  async def arr_set(self, reqName:str, rspName:str, name: str, pos: int, item: dict) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(reqName, rspName, {'name':name, 'pos': pos, 'item':item})


  async def arr_set_rng(self, reqName:str, rspName:str, name: str, pos: int, items: List[dict]) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(reqName, rspName, {'name':name, 'pos': pos, 'items':items})


  async def arr_get(self, reqName:str, rspName:str, name: str, pos: int) -> dict:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(reqName, rspName, {'name':name, 'pos':pos})
    return rsp[rspName]['item']


  async def arr_get_rng(self,reqName:str, rspName:str, name: str, start: int, stop = None) -> List[dict]:
    raise_if_empty(name)

    if stop == None:
      rng = [start]
    elif start > stop:
      raise ValueError('start > stop')
    else:
      rng = [start, stop]
        
    rsp = await self.client.sendCmd(reqName, rspName, {'name':name, 'rng':rng})
    return rsp[rspName]['items']
  
  
  async def arr_len(self, reqName:str, rspName:str, name: str) -> int:
    raise_if_empty(name)
    rsp = await self.client.sendCmd(reqName, rspName, {'name':name})
    return rsp[rspName]['len']


  async def arr_swap(self, reqName:str, rspName:str, name: str, posA: int, posB: int) -> int:
    raise_if_empty(name)
    await self.client.sendCmd(reqName, rspName, {'name':name, 'posA':posA, 'posB':posB})

  
  async def arr_exist(self, reqName:str, rspName:str, name: str) -> bool:
    # this query uses the status to indicate result: StValue.ST_SUCCESS means the array exists
    raise_if_empty(name)
    # checkStatus=False because 'status' not being success is not an error
    rsp = await self.client.sendCmd(reqName, rspName, {'name':name}, checkStatus=False)    
    return rsp[rspName]['st'] == StValues.ST_SUCCESS
  

  async def arr_clear(self, reqName:str, rspName:str, name: str, start: int, stop: int) -> None:
    raise_if_empty(name)
    await self.client.sendCmd(reqName, rspName, {'name':name, 'rng':[start, stop]})


#region OArrays
class OArrays:
  
  def __init__(self, client: NdbClient):
    self._array = _Arrays(client)


  async def oarr_create(self, *args) -> int:
    return await self._array.arr_create(OArrCmd.CREATE_REQ, OArrCmd.CREATE_RSP, *args)


  async def oarr_delete(self, *args) -> None:
    await self._array.arr_delete(OArrCmd.DELETE_REQ, OArrCmd.DELETE_RSP, *args)

  
  async def oarr_delete_all(self) -> None:
    await self._array.arr_delete_all(OArrCmd.DELETE_ALL_REQ, OArrCmd.DELETE_ALL_RSP)


  async def oarr_set(self, *args) -> None:
    await self._array.arr_set(OArrCmd.SET_REQ, OArrCmd.SET_RSP, *args)
    

  async def oarr_set_rng(self, *args) -> None:
    await self._array.arr_set_rng(OArrCmd.SET_RNG_REQ, OArrCmd.SET_RNG_RSP,*args)


  async def oarr_get(self, *args) -> dict:
    return await self._array.arr_get(OArrCmd.GET_REQ, OArrCmd.GET_RSP, *args)
       

  async def oarr_get_rng(self, *args) -> List[dict]:
    return await self._array.arr_get_rng(OArrCmd.GET_RNG_REQ, OArrCmd.GET_RNG_RSP, *args)
  
  
  async def oarr_len(self, *args) -> int:
    return await self._array.arr_len(OArrCmd.LEN_REQ, OArrCmd.LEN_RSP, *args)
  

  async def oarr_swap(self, *args) -> int:
    return await self._array.arr_swap(OArrCmd.SWAP_REQ, OArrCmd.SWAP_RSP, *args)

  
  async def oarr_exist(self, *args) -> bool:
    return await self._array.arr_exist(OArrCmd.EXIST_REQ, OArrCmd.EXIST_RSP, *args)
  

  async def oarr_clear(self, *args) -> None:
    return await self._array.arr_clear(OArrCmd.CLEAR_REQ, OArrCmd.CLEAR_RSP, *args)

#endregion


#region IArray

class IArrays:
  
  def __init__(self, client: NdbClient):
    self._array = _Arrays(client)

  async def iarr_create(self, *args) -> int:
    return await self._array.arr_create(IArrCmd.CREATE_REQ, IArrCmd.CREATE_RSP, *args)


  async def iarr_delete(self, *args) -> None:
    await self._array.arr_delete(IArrCmd.DELETE_REQ, IArrCmd.DELETE_RSP, *args)

  
  async def iarr_delete_all(self) -> None:
    await self._array.arr_delete_all(IArrCmd.DELETE_ALL_REQ, IArrCmd.DELETE_ALL_RSP)


  async def iarr_set(self, *args) -> None:
    await self._array.arr_set(IArrCmd.SET_REQ, IArrCmd.SET_RSP, *args)
    

  async def iarr_set_rng(self, *args) -> None:
    await self._array.arr_set_rng(IArrCmd.SET_RNG_REQ, IArrCmd.SET_RNG_RSP,*args)


  async def iarr_get(self, *args) -> dict:
    return await self._array.arr_get(IArrCmd.GET_REQ, IArrCmd.GET_RSP, *args)
       

  async def iarr_get_rng(self, *args) -> List[dict]:
    return await self._array.arr_get_rng(IArrCmd.GET_RNG_REQ, IArrCmd.GET_RNG_RSP, *args)
  
  
  async def iarr_len(self, *args) -> int:
    return await self._array.arr_len(IArrCmd.LEN_REQ, IArrCmd.LEN_RSP, *args)
  

  async def iarr_swap(self, *args) -> int:
    return await self._array.arr_swap(IArrCmd.SWAP_REQ, IArrCmd.SWAP_RSP, *args)

  
  async def iarr_exist(self, *args) -> bool:
    return await self._array.arr_exist(IArrCmd.EXIST_REQ, IArrCmd.EXIST_RSP, *args)
  

  async def iarr_clear(self, *args) -> None:
    return await self._array.arr_clear(IArrCmd.CLEAR_REQ, IArrCmd.CLEAR_RSP, *args)

#endregion