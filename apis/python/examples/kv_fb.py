

import asyncio as asio
import sys
import flatbuffers
import flatbuffers.flexbuffers
import builtins
sys.path.append('../')
from ndb.client import NdbClient
from ndb.fbs.ndb.request import Request, RequestBody, KVSet, KVGet
from ndb.fbs.ndb.response import Response, ResponseBody, Status, KVGet as KVGetRsp


def createKvArray(kv: dict) -> bytearray:
  
  # NOTE MapFromElements() does not differentiate between Int and UInt
  # so do serialising manually here
  b = flatbuffers.flexbuffers.Builder()

  def getValueTypeAndAdd(v):
    match type(v):
      case builtins.int:
        return b.Int if v < 0 else b.UInt
      
      case builtins.str:
        return b.String

      case builtins.float:
        return b.Float

      case builtins.bool:
        return b.Bool
    
      case _:
        return None


  with b.Map():
    for k,v in kv.items():
      f = getValueTypeAndAdd(v)
      if f:
        b.Key(k)
        f(v)

  return b.Finish()


async def test():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987')
  
  builder = flatbuffers.Builder()

  data = {'k1':123, 'k2':'hello', 'k3':123.456, 'k4':-123, 'k5':False,'k6':True}
  
  testBytes = createKvArray(data)

  kvVec = builder.CreateByteVector(testBytes)

  KVSet.Start(builder)
  KVSet.AddKv(builder, kvVec)
  body = KVSet.End(builder)

  Request.RequestStart(builder)
  Request.AddBodyType(builder, RequestBody.RequestBody.KVSet)
  Request.AddBody(builder, body)
  req = Request.RequestEnd(builder)

  builder.Finish(req)

  buffer = builder.Output()
  rsp = await client.sendCmd2(buffer)
  
  response = Response.Response.GetRootAs(rsp)
  

  if response.Status() is not Status.Status.Ok:
    print('Fail')
  else:
    print('Ok')

    builder.Clear()
    
    keys = ['k1', 'k2','k3','k4','k5''k6']
    keysOffsets = []

    for key in keys:
      keysOffsets.append(builder.CreateString(key))

    KVGet.StartKeysVector(builder, len(keysOffsets))
    for off in keysOffsets:
      builder.PrependUOffsetTRelative(off)
    
    keys = builder.EndVector()

    KVGet.Start(builder)
    KVGet.AddKeys(builder, keys)
    body = KVGet.End(builder)

    Request.Start(builder)
    Request.AddBodyType(builder, RequestBody.RequestBody.KVGet)
    Request.AddBody(builder, body)
    req = Request.End(builder)

    builder.Finish(req)

    buffer = builder.Output()

    rspBuffer = await client.sendCmd2(buffer)
    rsp = Response.Response.GetRootAs(rspBuffer)

    if rsp.Status() == Status.Status.Ok:
      if rsp.BodyType() == ResponseBody.ResponseBody.KVGet:
        union_body = KVGetRsp.KVGet()
        union_body.Init(rsp.Body().Bytes, rsp.Body().Pos)

        print(flatbuffers.flexbuffers.Loads(union_body.KvAsNumpy().tobytes()))
        


if __name__ == "__main__":
  for f in [test()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)