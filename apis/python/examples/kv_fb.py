

import asyncio as asio
import sys
import flatbuffers
import flatbuffers.flexbuffers
import builtins
sys.path.append('../')
from ndb.client import NdbClient
from ndb.kv import KV2
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
  
  data = {'k1':123, 'k2':'hello', 'k3':123.456, 'k4':-123, 'k5':False,'k6':True}
    
  kv = KV2(client)

  try:
    await kv.set(data)
    
    print(await kv.get(keys=['k1', 'k2','k3','k4','k5''k6']))

    await kv.remove(keys=['k1'])

    print(await kv.get(keys=['k1', 'k2','k3','k4','k5''k6']))
    
  except:
    print('Query failed')
        


if __name__ == "__main__":
  for f in [test()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)