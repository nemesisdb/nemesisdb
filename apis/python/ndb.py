

#!/usr/bin/env python

"""Client using the asio API."""


import json
import asyncio as asio
import logging
import signal
from websockets.asyncio.client import connect
#from websockets import ConnectionClosed

# logger = logging.getLogger('websockets')
# logger.setLevel(logging.DEBUG)
# logger.addHandler(logging.StreamHandler())


"""
The Api class runs the websockets library, handling asyncio
coroutines.
This class is not intended to be used directly, instead
use the Client class.
"""
class Api:
  
  def __init__(self):
    pass


  async def start(self, uri: str):    
    self.uri = uri
    self.connected = asio.Semaphore(0)
    self.rspEvt = asio.Event()
    self.listen_task = asio.create_task(self.open())
    await self.connected.acquire()
    return self.listen_task
  

  async def open(self):    
    async with connect(self.uri) as websocket:
      self.ws = websocket
      loop = asio.get_running_loop()
      loop.add_signal_handler(signal.SIGTERM, loop.create_task, self.close()) # these cause an 'unwaited' warning
      loop.add_signal_handler(signal.SIGINT, loop.create_task, self.close())
      self.connected.release()

      while True:        
        self.message = await asio.create_task(websocket.recv())
        self.rspEvt.set()


  async def query(self, s: dict, expectRsp: bool) -> dict:
    await asio.gather(asio.create_task(self._query(json.dumps(s), expectRsp)))   
    return json.loads(self.message) if expectRsp else None


  async def _query(self, s: str, expectRsp: bool):
    await self.ws.send(s, text=True)

    if expectRsp:
      await self.rspEvt.wait()
      self.rspEvt.clear()


  async def close(self):
    print("Closing")
    await self.ws.close()


_STATUS = 'st'
_CMD_SET = 'KV_SET'
_RSP_SET = 'KV_SET_RSP'
_CMD_ADD = 'KV_ADD'
_RSP_ADD = 'KV_ADD_RSP'
_CMD_GET = 'KV_GET'
_RSP_GET = 'KV_GET_RSP'
_CMD_RMV = 'KV_RMV'
_RSP_RMV = 'KV_RMV_RSP'
_CMD_COUNT = 'KV_COUNT'
_RSP_COUNT = 'KV_COUNT_RSP'
_CMD_CONTAINS = 'KV_CONTAINS'
_RSP_CONTAINS = 'KV_CONTAINS_RSP'
_CMD_CLEAR = 'KV_CLEAR'
_RSP_CLEAR = 'KV_CLEAR_RSP'
_CMD_CLEAR_SET = 'KV_CLEAR_SET'
_RSP_CLEAR_SET = 'KV_CLEAR_SET_RSP'
_CMD_KEYS = 'KV_KEYS'
_RSP_KEYS = 'KV_KEYS_RSP'



class Client:
  
  def __init__(self, uri: str):
    self.uri = uri
    self.api = Api()


  async def listen(self):
    return await self.api.start(self.uri)


  async def set(self, keys: dict) -> bool:
    return await self._doSetAdd(_CMD_SET, _RSP_SET, keys)
  

  async def add(self, keys: dict) -> bool:
    return await self._doSetAdd(_CMD_ADD, _RSP_ADD, keys)


  async def get(self, keys: tuple) -> dict:
    q = {_CMD_GET : {'keys':keys}}
    rsp = await self.api.query(q, True)
    return (self.isRspSuccess(rsp, _RSP_GET), rsp[_RSP_GET]['keys'])
  

  async def rmv(self, keys: tuple) -> dict:
    q = {_CMD_RMV : {'keys':keys}}
    rsp = await self.api.query(q, True)
    return self.isRspSuccess(rsp, _RSP_RMV)


  async def count(self) -> tuple:
    q = {_CMD_COUNT : {}}
    rsp = await self.api.query(q, True)
    return (self.isRspSuccess(rsp, _RSP_COUNT), rsp[_RSP_COUNT]['cnt'])


  async def contains(self, keys: tuple) -> tuple:
    q = {_CMD_CONTAINS : {'keys':keys}}
    rsp = await self.api.query(q, True)
    return (self.isRspSuccess(rsp, _RSP_CONTAINS), rsp[_RSP_CONTAINS]['contains'])

  
  async def keys(self) -> tuple:
    q = {_CMD_KEYS : {}}
    rsp = await self.api.query(q, True)
    return (self.isRspSuccess(rsp, _RSP_KEYS), rsp[_RSP_KEYS]['keys'])


  async def clear(self) -> tuple:
    q = {_CMD_CLEAR : {}}
    rsp = await self.api.query(q, True)
    return (self.isRspSuccess(rsp, _RSP_CLEAR), rsp[_RSP_CLEAR]['cnt'])


  async def clear_set(self, keys: dict) -> tuple:
    q = {_CMD_CLEAR_SET : {'keys':keys}}
    rsp = await self.api.query(q, True)
    return (self.isRspSuccess(rsp, _RSP_CLEAR_SET), rsp[_RSP_CLEAR_SET]['cnt'])


  def isRspSuccess(self, rsp: dict, cmd: str) -> bool:
    return rsp[cmd][_STATUS] == 1
    

  async def close(self):  ##### TODO test
    await self.api.close()


  async def _doSetAdd(self, cmdName: str, rspName: str, keys: dict) -> bool:
    q = {cmdName : {'keys':keys}}   # cmdName is either KV_SET or KV_ADD
    rsp = await self.api.query(q, True)
    return self.isRspSuccess(rsp, rspName)



async def do_stuff():
  client = Client("ws://127.0.0.1:1987/")
  listen_task = await client.listen()

  max = 10


  # clear before test
  await client.clear()  


  # set
  for i in range(0,max):
    ok = await client.set({f"username{i}":f"user{i}", f"password{i}":f"passy{i}"})
    assert ok

  for i in range(0,max):
    (ok, rsp) = await client.get((f"username{i}",f"password{i}"))
    assert ok


  # get
  usernameKeys = tuple()
  passwordKeys = tuple()
  for i in range(0,max):
    usernameKeys = usernameKeys + (f'username{i}',)
    passwordKeys = passwordKeys + (f'password{i}',)

  (ok, keys) = await client.get(usernameKeys)
  assert ok and len(keys) == max
  
  (ok, keys) = await client.get(passwordKeys)
  assert ok and len(keys) == max


  # add - don't overwrite existing key
  ok = await client.add({'username0':'should_not_update'})
  assert ok
  (ok, keys) = await client.get(('username0',))
  assert keys['username0'] == 'user0'  # unchanged from set above

  # add - new key
  ok = await client.add({f'username{max}':'abc'})
  assert ok
  (ok, keys) = await client.get((f'username{max}',))
  assert keys[f'username{max}'] == 'abc'  # added

  
  # rmv - remove key stored with add
  ok = await client.rmv((f'username{max}',))
  assert ok
  (ok, keys) = await client.get((f'username{max}',))
  assert len(keys) == 0 # key not found because we removed it
  

  # count
  (ok, count) = await client.count()
  assert ok and count == max*2  # *2 because we add usernameN and passwordN for range(0,max)
  

  # contains - first two keys exist, third key does not
  (ok, keys) = await client.contains(('username0','username3', f'username{max*2}'))
  # keys only contains keys that exist
  assert (ok and
          len(keys) == 2 and
          f'username{max*2}' not in keys)

  
  # clear_set
  (ok, count) = await client.clear_set({'k1':'v1','k2':'v2','k3':'v3','k4':'v4'})
  assert ok and count == max*2  # clear_set() returns number of keys cleared
  
    
  # keys
  (ok, keys) = await client.keys()
  assert ok and len(keys) == 4 and keys == ['k1','k2','k3','k4']


  # clear the keys set with clear_set
  (ok, count) = await client.clear()
  assert ok and count == 4

  
  await listen_task


if __name__ == "__main__":
  asio.run(do_stuff())

