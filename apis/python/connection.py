import json
import logging
import asyncio as asio
from asyncio import CancelledError
from websockets.asyncio.client import connect
from websockets import ConnectionClosed

# logger = logging.getLogger('websockets')
# logger.setLevel(logging.DEBUG)
# logger.addHandler(logging.StreamHandler())


"""
The Api class runs the websockets library, handling asyncio
coroutines.
This class is not intended to be used directly, instead
use the Client class.
"""
class Connection:

  def __init__(self):
    pass


  async def start(self, uri: str):    
    self.uri = uri
    self.userClosed = False    
    self.rspEvt = asio.Event()
    self.connectedSem = asio.Semaphore(0)
    self.rcvTask = None
    self.queryTask = None
    self.listen_task = asio.create_task(self.open())
    await self.connectedSem.acquire()

    return self.listen_task # TODO does this need to be returned? probably not
  

  async def open(self):
    try:
      async with connect(self.uri, open_timeout=5) as websocket:
        self.ws = websocket
        self.connectedSem.release()

        while True: ## TODO ! uncomment !
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
      
      if self.queryTask != None:
        self.queryTask.cancel()

      if self.userClosed == False:
        self.rcvTask.cancel()
      
      
  async def query(self, s: dict, expectRsp = True) -> dict:
    self.queryTask = asio.create_task(self._query(json.dumps(s), expectRsp))   

    # if there is an active query when we are disconnected, the query
    # task is cancelled, raising an exception.
    try:
      await self.queryTask
    except asio.CancelledError:
      pass

    return json.loads(self.message) if expectRsp else None


  async def _query(self, s: str, expectRsp: bool):
    await self.ws.send(s, text=True)

    if expectRsp:
      await self.rspEvt.wait()
      self.rspEvt.clear()


  async def close(self):
    self.userClosed = True
    await self.ws.close()

