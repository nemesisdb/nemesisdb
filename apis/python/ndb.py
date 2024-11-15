

#!/usr/bin/env python


import json
import logging
import asyncio as asio
from typing import Tuple
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
class Api:

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

    return self.listen_task
  

  async def open(self):
    try:
      async with connect(self.uri, open_timeout=5) as websocket:
        self.ws = websocket
        self.connectedSem.release()

        while True: ## TODO ! uncomment !
          self.rcvTask = asio.create_task(websocket.recv(), name='recv')
          # self.rcvTask.add_done_callback()
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
      
      
  async def query(self, s: dict, expectRsp: bool) -> dict:
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


"""
  Represents a client, use to query the database.
  This classes uses the Api class so that only relevant values
  are returned.

  - The query functions will return when the response is received (as opposed
    to a callback handler to receive all responses). This serialises responses and
    should simplify user code.
  - This design design means that KV_SETQ or KV_ADDQ are not supported.
"""
class Client:
  
  def __init__(self):
    self.uri = ''
    self.listen_task = None
    self.api = Api()


  async def listen(self, uri: str):
    if uri == '':
      raise ValueError('No URI')
    
    self.uri = uri
    self.listen_task = await self.api.start(self.uri)
    return self.listen_task


  async def set(self, keys: dict) -> bool:
    return await self._doSetAdd(_CMD_SET, _RSP_SET, keys)
  

  async def add(self, keys: dict) -> bool:
    return await self._doSetAdd(_CMD_ADD, _RSP_ADD, keys)


  async def get(self, keys: tuple) -> Tuple[bool, dict]:
    q = {_CMD_GET : {'keys':keys}}
    rsp = await self.api.query(q, True)
    return (self._isRspSuccess(rsp, _RSP_GET), rsp[_RSP_GET]['keys'])
  

  async def rmv(self, keys: tuple) -> dict:
    q = {_CMD_RMV : {'keys':keys}}
    rsp = await self.api.query(q, True)
    return self._isRspSuccess(rsp, _RSP_RMV)


  async def count(self) -> tuple:
    q = {_CMD_COUNT : {}}
    rsp = await self.api.query(q, True)
    return (self._isRspSuccess(rsp, _RSP_COUNT), rsp[_RSP_COUNT]['cnt'])


  async def contains(self, keys: tuple) -> tuple:
    q = {_CMD_CONTAINS : {'keys':keys}}
    rsp = await self.api.query(q, True)
    return (self._isRspSuccess(rsp, _RSP_CONTAINS), rsp[_RSP_CONTAINS]['contains'])

  
  async def keys(self) -> tuple:
    q = {_CMD_KEYS : {}}
    rsp = await self.api.query(q, True)
    return (self._isRspSuccess(rsp, _RSP_KEYS), rsp[_RSP_KEYS]['keys'])


  async def clear(self) -> tuple:
    q = {_CMD_CLEAR : {}}
    rsp = await self.api.query(q, True)
    return (self._isRspSuccess(rsp, _RSP_CLEAR), rsp[_RSP_CLEAR]['cnt'])


  async def clear_set(self, keys: dict) -> tuple:
    q = {_CMD_CLEAR_SET : {'keys':keys}}
    rsp = await self.api.query(q, True)
    return (self._isRspSuccess(rsp, _RSP_CLEAR_SET), rsp[_RSP_CLEAR_SET]['cnt'])


  async def close(self):
    await self.api.close()


  def get_listen_task(self) -> asio.Task:
    return self.listen_task
  

  def _isRspSuccess(self, rsp: dict, cmd: str) -> bool:
    return rsp[cmd][_STATUS] == 1


  async def _doSetAdd(self, cmdName: str, rspName: str, keys: dict) -> bool:
    q = {cmdName : {'keys':keys}}   # cmdName is either KV_SET or KV_ADD
    rsp = await self.api.query(q, True)
    return self._isRspSuccess(rsp, rspName)

