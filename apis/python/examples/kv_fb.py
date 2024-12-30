

import asyncio as asio
import sys
import flatbuffers
import builtins
sys.path.append('../')
from ndb.client import NdbClient
from ndb.fbs.ndb.request import Request, KV, ValueType, Int64, String


def createInt64 (b: flatbuffers.Builder, v: Int64) -> int:
  Int64.Start(b)
  Int64.AddVal(b, v)
  return Int64.End(b)


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
        val = createInt64(b, v)
        t = ValueType.ValueType.Int64
      
      case builtins.str:
        val = createString(b,v)
        t = ValueType.ValueType.String

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

  Request.StartKvVector(b, len(kvObjectsOffsets))

  for offset in kvObjectsOffsets:
    b.PrependUOffsetTRelative(offset)
  return b.EndVector()


async def test():
  builder = flatbuffers.Builder()

  #k1 = builder.CreateString('k1')
  
  # Int64.Start(builder)
  # Int64.AddVal(builder, 123)
  # val = Int64.End(builder)
  
  # KV.KVStart(builder)
  # KV.AddKey(builder, k1)
  # KV.AddValType(builder, ValueType.ValueType.Int64)
  # KV.KVAddVal(builder, val)
  # KV.KVEnd(builder)
  data = {'k1':123, 'k2':456, 'k3':'hello'}
  key_vals = createKVVector(builder, data)
  
  
  # Request.StartKvVector(builder, len(data))
  # builder.PrependUOffsetTRelative(key_vals)
  # vec = builder.EndVector()

  Request.RequestStart(builder)
  Request.AddKv(builder, key_vals)
  req = Request.RequestEnd(builder)

  builder.Finish(req ,file_identifier=bytearray('KV  '.encode()))
  buffer = builder.Output()

  client = NdbClient()
  await client.open('ws://127.0.0.1:1987')
  await client.sendCmd2(buffer)

  


if __name__ == "__main__":
  for f in [test()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)