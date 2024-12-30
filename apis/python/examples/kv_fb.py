

import asyncio as asio
import sys
import flatbuffers
import builtins
sys.path.append('../')
from ndb.client import NdbClient
from ndb.fbs.ndb.request import Request, KV, ValueType, Int64, UInt64, String, Double, Bool, RequestBody, KVSet


def createInteger (b: flatbuffers.Builder, v: int) -> int:
  if v < 0:
    Int64.Start(b)
    Int64.AddVal(b, v)
    return Int64.End(b)
  else:
    UInt64.Start(b)
    UInt64.AddVal(b, v)
    return UInt64.End(b)


def createBool (b: flatbuffers.Builder, v: bool) -> int:
  Bool.Start(b)
  Bool.AddVal(b, v)
  return Bool.End(b)


def createDouble (b: flatbuffers.Builder, v: float) -> int:
  Double.Start(b)
  Double.AddVal(b, v)
  return Double.End(b)


def createString (b: flatbuffers.Builder, v: str) -> str:
  s = b.CreateString(v)
  String.Start(b)
  String.AddVal(b, s)
  return String.End(b)


def createKVVector(b: flatbuffers.Builder, kv: dict) -> int:
  
  keyOffsets = []
  valueOffsets = []
  typeOffsets = []

  for k in kv:
    keyOffsets.append(b.CreateString(k))
  
  for k,v in kv.items():
    match type(v):
      case builtins.int:
          val = createInteger(b, v)
          t = ValueType.ValueType.Int64
      
      case builtins.str:
        val = createString(b,v)
        t = ValueType.ValueType.String

      case builtins.float:
        val = createDouble(b,v)
        t = ValueType.ValueType.Double

      case builtins.bool:
        val = createBool(b,v)
        t = ValueType.ValueType.Bool

      case _:
        t = None

    if t is not None:
      valueOffsets.append(val)
      typeOffsets.append(t)
  
  assert (len(keyOffsets) == len(typeOffsets) == len(valueOffsets))

  kvObjectsOffsets = []

  i = 0
  for k in keyOffsets:
    KV.KVStart(b)
    KV.AddKey(b, keyOffsets[i])
    KV.AddValType(b, typeOffsets[i])
    KV.AddVal(b, valueOffsets[i])
    kvObjectsOffsets.append(KV.KVEnd(b))
    
    i += 1

  KVSet.StartKvVector(b, len(kvObjectsOffsets))

  for offset in kvObjectsOffsets:
    b.PrependUOffsetTRelative(offset)
  return b.EndVector()


async def test():
  builder = flatbuffers.Builder()

  data = {'k1':123, 'k2':'hello', 'k3':123.456, 'k4':-123, 'k5':False,'k6':True}
  key_vals = createKVVector(builder, data)
  
  KVSet.Start(builder)
  KVSet.AddKv(builder, key_vals)
  body = KVSet.End(builder)

  Request.RequestStart(builder)
  Request.AddBodyType(builder, RequestBody.RequestBody.KVSet)
  Request.AddBody(builder, body)
  req = Request.RequestEnd(builder)

  builder.Finish(req)
  buffer = builder.Output()

  client = NdbClient()
  await client.open('ws://127.0.0.1:1987')
  await client.sendCmd2(buffer)

  


if __name__ == "__main__":
  for f in [test()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)