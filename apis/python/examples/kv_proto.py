import asyncio as asio
import sys
from google.protobuf.struct_pb2 import Struct

sys.path.append('../')
from ndb.client import NdbClient
from ndb.proto import set_pb2 as Set


def packMap (map: Struct, kv: dict):
  for k,v in kv.items():
    map[k] = v


async def test():
  
  req = Set.Request()

  packMap(req.set.kv, {'s':'string', 'ui':12312, 'si':-12312, 'lst':['a','b','c']})

  print(req)

  client = NdbClient()
  await client.open('ws://127.0.0.1:1987')

  await client.sendCmd2(req.SerializeToString())


if __name__ == "__main__":
  for f in [test()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)