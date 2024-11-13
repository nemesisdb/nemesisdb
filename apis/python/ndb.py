

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


class ndb_api:
  
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
      loop.add_signal_handler(signal.SIGTERM, loop.create_task, self.close())
      loop.add_signal_handler(signal.SIGINT, loop.create_task, self.close())
      self.connected.release()

      while True:        
        self.message = await asio.create_task(websocket.recv())
        self.rspEvt.set()


  async def query(self, s: str, expectRsp: bool) -> dict:
    await asio.gather(asio.create_task(self._query(s, expectRsp)))   
    return json.loads(self.message) if expectRsp else None


  async def _query(self, s: str, expectRsp: bool):
    await self.ws.send(s, text=True)

    if expectRsp:
      await self.rspEvt.wait()
      self.rspEvt.clear()


  async def close(self):
    print("Closing")
    await self.ws.close()



class Client:
  
  def __init__(self, uri: str):
    self.uri = uri
    self.api = ndb_api()


  async def listen(self):
    tsk = await self.api.start(self.uri)
    return tsk


  async def set(self, keys: dict) -> bool:
    q = {"KV_SET" : {"keys":keys}}
    rsp = await self.api.query(json.dumps(q), True)
    return self.isRspSuccess(rsp, 'KV_SET_RSP')

  async def get(self, keys: tuple) -> dict:
    q = {"KV_GET" : {"keys":keys}}
    rsp = await self.api.query(json.dumps(q), True)
    return (self.isRspSuccess(rsp, 'KV_GET_RSP'), rsp)

  def isRspSuccess(self, rsp: dict, cmd: str) -> bool:
    return rsp[cmd]['st'] == 1
    


async def do_stuff():
  client = Client("ws://127.0.0.1:1987/")
  listen_task = await client.listen()

  for i in range(0,10):
    ok = await client.set({"username":f"desire{i}"})
    assert ok, "set"

  (ok, rsp) = await client.get(("username",))
  assert ok, "get"
  print(rsp)

  await listen_task


if __name__ == "__main__":
  asio.run(do_stuff())

