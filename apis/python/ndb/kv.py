from ndb.commands import (StValues, KvCmds)
from ndb.client import NdbClient
from ndb.common import raise_if_empty, raise_if_not, raise_if, CreateKvArray
from typing import List, Any
import flatbuffers
import flatbuffers.flexbuffers
from ndb.fbs.ndb.request import Request, RequestBody, KVSet, KVGet
from ndb.fbs.ndb.response import Response, ResponseBody, Status, KVGet as KVGetRsp


class KV2:
  "Key Value"


  def __init__(self, client: NdbClient):
    self.client = client


  async def set(self, kv: dict) -> None:
    raise_if(len(kv) == 0, 'keys empty')

    try:
      fb = flatbuffers.Builder()
      kvVec = fb.CreateByteVector(CreateKvArray(kv))

      KVSet.Start(fb)
      KVSet.AddKv(fb, kvVec)
      body = KVSet.End(fb)

      self._completeRequest(fb, body, RequestBody.RequestBody.KVSet)

      await self.client.sendCmd2(fb.Output())
    finally:
      fb.Clear()

  
  async def get(self, key=None, keys=[]) -> dict:
    raise_if(key is None and len(keys) == 0, 'key or keys must be set')

    if len(keys) == 0:
      keys = [key]

    try:
      fb = flatbuffers.Builder()
      keysOff = self._createStrings(fb, keys)

      KVGet.Start(fb)
      KVGet.AddKeys(fb, keysOff)
      body = KVGet.End(fb)

      self._completeRequest(fb, body, RequestBody.RequestBody.KVGet)
      
      rspBuffer = await self.client.sendCmd2(fb.Output())

      rsp = Response.Response.GetRootAs(rspBuffer)
      if rsp.BodyType() == ResponseBody.ResponseBody.KVGet:
        union_body = KVGetRsp.KVGet()
        union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)
        # this is how we get a flexbuffer from a flatbuffer
        return flatbuffers.flexbuffers.Loads(union_body.KvAsNumpy().tobytes())

    finally:
      fb.Clear()


  def _createStrings (self, fb: flatbuffers.Builder, strings: list) -> int:
    keysOffsets = []
    for key in strings:
      keysOffsets.append(fb.CreateString(key))
    
    fb.StartVector(4, len(strings), 4)
    for off in keysOffsets:
        fb.PrependUOffsetTRelative(off)
    return fb.EndVector()
    

  def _completeRequest(self, fb: flatbuffers.Builder, body: int, bodyType: RequestBody.RequestBody):
    try:
      Request.RequestStart(fb)
      Request.AddBodyType(fb, bodyType)
      Request.AddBody(fb, body)
      req = Request.RequestEnd(fb)

      fb.Finish(req)
    except:
      fb.Clear()



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
