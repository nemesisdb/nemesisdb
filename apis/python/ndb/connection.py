import json
import asyncio as asio
from asyncio import CancelledError
from websockets.asyncio.client import connect
from websockets import ConnectionClosed


# TODO could allow enable of websockets logger here

class Connection:
  """
  The Api class runs the websockets library, handling asyncio coroutines.
  """

  def __init__(self):
    pass


  async def start(self, uri: str) -> None:
    self.uri = uri
    self.userClosed = False
    self.opened = False     
    self.rcvTask = None
    self.ws = None
    self.rspEvt = asio.Event()
    self.connectedSem = asio.Semaphore(0)
    self.listen_task = asio.create_task(self.open())
    
    # the listen_task will release this semaphore
    await self.connectedSem.acquire()
    if not self.opened:
      raise ConnectionClosed(f'Failed connection to {uri}')
  

  async def open(self):
    try:
      async with connect(self.uri, open_timeout=5) as websocket:
        self.ws = websocket
        self.opened = True
        self.connectedSem.release()

        while True:
          self.rcvTask = asio.create_task(websocket.recv(), name='recv')
          self.message = await self.rcvTask
          self.rspEvt.set()
    except OSError:
      # failed to connect
      if self.connectedSem.locked():
        self.connectedSem.release()
    except (ConnectionClosed, CancelledError):
      pass
    finally:
      self.rspEvt.clear()
      self.opened = False

      if self.userClosed == False and self.rcvTask != None:
        self.rcvTask.cancel()


  async def query2(self, buffer: bytearray) -> bytes:
    try:
      queryTask = asio.create_task(self._query2(buffer))
      await queryTask
      msg = queryTask.result()
    except asio.CancelledError:
      # if there is an active query when we are disconnected, the query
      # task is cancelled, raising an exception
      pass
    
    return msg


  async def _query2(self, buffer: bytearray) -> bytes:
    await self.ws.send(buffer, text=False)    
    await self.rspEvt.wait()
    msg = self.message    
    self.rspEvt.clear()
    return msg
  

  async def query(self, s: dict) -> dict:
    try:
      queryTask = asio.create_task(self._query(json.dumps(s)))
      await queryTask
      msg = queryTask.result()
    except asio.CancelledError:
      # if there is an active query when we are disconnected, the query
      # task is cancelled, raising an exception
      pass

    return msg
  

  async def _query(self, query: str):
    await self.ws.send(query, text=True)    
    await self.rspEvt.wait()
    msg = json.loads(self.message)
    self.rspEvt.clear()
    return msg


  async def close(self):
    self.userClosed = True
    if self.ws:
      await self.ws.close()

