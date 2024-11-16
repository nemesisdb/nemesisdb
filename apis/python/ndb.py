import connection
import asyncio as asio
from connection import Connection
from typing import Tuple, List
from asyncio import CancelledError
from websockets.asyncio.client import connect
from websockets import ConnectionClosed


class Fields:
  STATUS    = 'st'


class KvCmd:
  SET_REQ       = 'KV_SET'
  SET_RSP       = 'KV_SET_RSP'
  ADD_REQ       = 'KV_ADD'
  ADD_RSP       = 'KV_ADD_RSP'
  GET_REQ       = 'KV_GET'
  GET_RSP       = 'KV_GET_RSP'
  RMV_REQ       = 'KV_RMV'
  RMV_RSP       = 'KV_RMV_RSP'
  COUNT_REQ     = 'KV_COUNT'
  COUNT_RSP     = 'KV_COUNT_RSP'
  CONTAINS_REQ  = 'KV_CONTAINS'
  CONTAINS_RSP  = 'KV_CONTAINS_RSP'
  CLEAR_REQ     = 'KV_CLEAR'
  CLEAR_RSP     = 'KV_CLEAR_RSP'
  CLEAR_SET_REQ = 'KV_CLEAR_SET'
  CLEAR_SET_RSP = 'KV_CLEAR_SET_RSP'
  KEYS_REQ      = 'KV_KEYS'
  KEYS_RSP      = 'KV_KEYS_RSP'



"""Represents a client, use to query the database.
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
    self.api = Connection()


  async def listen(self, uri: str):
    if uri == '':
      raise ValueError('No URI')
    
    self.uri = uri
    self.listen_task = await self.api.start(self.uri)
    return self.listen_task


  async def set(self, keys: dict, tkn = 0) -> bool:
    return await self._doSetAdd(KvCmd.SET_REQ, KvCmd.SET_RSP, keys, tkn)
  

  async def add(self, keys: dict, tkn = 0) -> bool:
    return await self._doSetAdd(KvCmd.ADD_REQ, KvCmd.ADD_RSP, keys, tkn)


  async def get(self, keys: tuple, tkn = 0) -> Tuple[bool, dict]:
    q = {KvCmd.GET_REQ : {'keys':keys}}
    #rsp = await self._send_kv_query(KvCmd.GET_REQ, q)
    rsp = await self._send_query(KvCmd.GET_REQ, q, tkn)
    return (self._is_rsp_valid(rsp, KvCmd.GET_RSP), rsp[KvCmd.GET_RSP]['keys'])
  

  async def rmv(self, keys: tuple, tkn = 0) -> dict:
    q = {KvCmd.RMV_REQ : {'keys':keys}}
    rsp = await self._send_query(KvCmd.RMV_REQ, q, tkn)
    return self._is_rsp_valid(rsp, KvCmd.RMV_RSP)


  async def count(self, tkn = 0) -> tuple:
    q = {KvCmd.COUNT_REQ : {}}
    rsp = await self._send_query(KvCmd.COUNT_REQ, q, tkn)

    if self._is_rsp_valid(rsp, KvCmd.COUNT_RSP):
      return (True, rsp[KvCmd.COUNT_RSP]['cnt'])
    else:
      return (False, 0)


  async def contains(self, keys: tuple, tkn = 0) -> tuple:
    q = {KvCmd.CONTAINS_REQ : {'keys':keys}}
    rsp = await self._send_query(KvCmd.CONTAINS_REQ, q, tkn)

    if self._is_rsp_valid(rsp, KvCmd.CONTAINS_RSP):
      return (True, rsp[KvCmd.CONTAINS_RSP]['contains'])
    else:
      (False, [])

  
  async def keys(self, tkn = 0) -> tuple:
    q = {KvCmd.KEYS_REQ : {}}
    rsp = await self._send_query(KvCmd.KEYS_REQ, q, tkn)
    
    if self._is_rsp_valid(rsp, KvCmd.KEYS_RSP):
      return (True, rsp[KvCmd.KEYS_RSP]['keys'])
    else:
      return (False, [])


  async def clear(self, tkn = 0) -> tuple:
    q = {KvCmd.CLEAR_REQ : {}}
    rsp = await self._send_query(KvCmd.CLEAR_REQ, q, tkn)
    
    if self._is_rsp_valid(rsp, KvCmd.CLEAR_RSP):
      return (True, rsp[KvCmd.CLEAR_RSP]['cnt'])
    else:
      return (False, [])
    

  async def clear_set(self, keys: dict, tkn = 0) -> tuple:
    q = {KvCmd.CLEAR_SET_REQ : {'keys':keys}}
    rsp = await self._send_query(KvCmd.CLEAR_SET_REQ, q, tkn)    
    return (self._is_rsp_valid(rsp, KvCmd.CLEAR_SET_RSP), rsp[KvCmd.CLEAR_SET_RSP]['cnt'])


  async def close(self):
    await self.api.close()


  def get_listen_task(self) -> asio.Task:
    return self.listen_task
  

  def _is_rsp_valid(self, rsp: dict, cmd: str) -> bool:
    return rsp[cmd][Fields.STATUS] == 1


  async def _doSetAdd(self, cmdName: str, rspName: str, keys: dict, tkn: int) -> bool:
    q = {cmdName : {'keys':keys}}   # cmdName is either KV_SET or KV_ADD
    rsp = await self._send_query(cmdName, q, tkn)
    return self._is_rsp_valid(rsp, rspName)
  
  
  async def _send_query(self, cmd: str, q: dict, tkn = 0):
    if tkn != 0:
      return await self._send_session_query(cmd, q, tkn)
    else:
      return await self._send_kv_query(q)


  async def _send_kv_query(self, q: dict):
    return await self.api.query(q)
  

  async def _send_session_query(self, cmd: str, q: dict, tkn: int):
    q[cmd]['tkn'] = tkn
    return await self.api.query(q)
    


"""This extends Client for session specific functionality. 
Which is nothing at the moment. SessionClient and Client are
not tied to a particular session, so they don't store the token.
"""
class SessionClient(Client):
  def __init__(self):
    super().__init__()


"""Stores the token and client on which the session was created.
"""
class Session:
  def __init__(self, tkn: int, client: SessionClient):
    self.tkn = tkn
    self.client = client
  

# Top level session functions. These functions are not tied to a 
# particular session, exact session_info() and end_session(),
# but there is no other logical place for them.

async def create_session(client: SessionClient) -> Session:
  # 'name' will be moved to a new SH_NEW_SHARED command
  q = {'SH_NEW':{'name':'default'}}
  rsp = await client._send_query('SH_NEW', q, 0)
  if client._is_rsp_valid(rsp, 'SH_NEW_RSP'):
    return Session(rsp['SH_NEW_RSP']['tkn'], client)
  return None


async def end_session(session: Session):
  q = {'SH_END':{}}
  rsp = await session.client._send_session_query('SH_END', q, session.tkn)
  return session.client._is_rsp_valid(rsp, 'SH_END_RSP')


async def session_exists(session: Session, tkns: List[int]) -> Tuple[bool, List]:
  q = {'SH_EXISTS':{'tkns':tkns}}
  rsp = await session.client._send_query('SH_EXISTS', q, 0)
  return (session.client._is_rsp_valid(rsp, 'SH_EXISTS_RSP'), rsp['SH_EXISTS_RSP']['exist'])


async def session_info(session: Session) -> dict:
  q = {'SH_INFO':{}}
  rsp = await session.client._send_session_query('SH_INFO', q, session.tkn)
  return (session.client._is_rsp_valid(rsp, 'SH_INFO_RSP'), rsp['SH_INFO_RSP'])


async def session_info_all(session: Session) -> dict:
  q = {'SH_INFO_ALL':{}}
  rsp = await session.client._send_session_query('SH_INFO_ALL', q, session.tkn)
  return (session.client._is_rsp_valid(rsp, 'SH_INFO_ALL_RSP'), rsp['SH_INFO_ALL_RSP'])


async def end_all_sessions(client: SessionClient) -> Tuple[bool, int]:
  q = {'SH_END_ALL':{}}
  rsp = await client._send_session_query('SH_END_ALL', q, 0)  # set tkn to 0, it's ignored
  return (client._is_rsp_valid(rsp, 'SH_END_ALL_RSP'), rsp['SH_END_ALL_RSP']['cnt'])
  



