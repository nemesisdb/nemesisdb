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
    rsp = await self._send_query(KvCmd.GET_REQ, q, tkn)
    if self._is_rsp_valid(rsp, KvCmd.GET_RSP):
      return (True, rsp[KvCmd.GET_RSP]['keys'])
    else:
      return (False, dict())
  

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
    rsp = await self._send_query(KvCmd.KEYS_REQ, {KvCmd.KEYS_REQ : {}}, tkn)
    
    if self._is_rsp_valid(rsp, KvCmd.KEYS_RSP):
      return (True, rsp[KvCmd.KEYS_RSP]['keys'])
    else:
      return (False, [])


  async def clear(self, tkn = 0) -> tuple:
    rsp = await self._send_query(KvCmd.CLEAR_REQ, {KvCmd.CLEAR_REQ : {}}, tkn)
    
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
  


class SessionCmd:
  NEW_REQ       = 'SH_NEW'
  NEW_RSP       = 'SH_NEW_RSP'
  END_REQ       = 'SH_END'
  END_RSP       = 'SH_END_RSP'
  END_ALL_REQ   = 'SH_END_ALL'
  END_ALL_RSP   = 'SH_END_ALL_RSP'
  EXISTS_REQ    = 'SH_EXISTS'
  EXISTS_RSP    = 'SH_EXISTS_RSP'
  INFO_REQ      = 'SH_INFO'
  INFO_RSP      = 'SH_INFO_RSP'
  INFO_ALL_REQ  = 'SH_INFO_ALL'
  INFO_ALL_RSP  = 'SH_INFO_ALL_RSP'


# Top level session functions. These functions are not tied to a 
# particular session, except session_info() and end_session(),
# but there is no other logical place for them.

"""Create a new session, with optional expiry settings.
expirySeconds - after this duration (seconds), the session expires. Default 0 - never expires.
deleteSessionOnExpire - when True, the sessions is deleted. When false, the session is not deleted. 

When a session expires, the keys are always deleted, but deleteSessionOnExpire controls if the 
actual session is also deleted.
"""
async def create_session(client: SessionClient, expirySeconds = 0, deleteSessionOnExpire = False) -> Session:
  # TODO 'name' will be optional
  # TODO add SH_NEW_SHARED command 
  q = {SessionCmd.NEW_REQ:{'name':'default'}}

  if expirySeconds < 0:
    raise ValueError('expirySeconds must be >= 0')
  
  if expirySeconds > 0:
    q[SessionCmd.NEW_REQ]['expiry'] = dict({'duration':expirySeconds, 'deleteSession':deleteSessionOnExpire})
    
  rsp = await client._send_query(SessionCmd.NEW_REQ, q)
  if client._is_rsp_valid(rsp, SessionCmd.NEW_RSP):
    return Session(rsp[SessionCmd.NEW_RSP]['tkn'], client)
  return None


async def end_session(session: Session):
  rsp = await session.client._send_session_query(SessionCmd.END_REQ, {SessionCmd.END_REQ:{}}, session.tkn)
  return session.client._is_rsp_valid(rsp, SessionCmd.END_RSP)


async def session_exists(session: Session, tkns: List[int]) -> Tuple[bool, List]:
  # when tkn is 0, _send_query() will not set the 'tkn'
  rsp = await session.client._send_query(SessionCmd.EXISTS_REQ, {SessionCmd.EXISTS_REQ:{'tkns':tkns}})
  return (session.client._is_rsp_valid(rsp, SessionCmd.EXISTS_RSP), rsp[SessionCmd.EXISTS_RSP]['exist'])


async def session_info(session: Session) -> dict:
  rsp = await session.client._send_session_query(SessionCmd.INFO_REQ, {SessionCmd.INFO_REQ:{}}, session.tkn)
  return (session.client._is_rsp_valid(rsp, SessionCmd.INFO_RSP), rsp[SessionCmd.INFO_RSP])


async def session_info_all(session: Session) -> dict:
  rsp = await session.client._send_session_query(SessionCmd.INFO_ALL_REQ, {SessionCmd.INFO_ALL_REQ:{}}, session.tkn)
  return (session.client._is_rsp_valid(rsp, SessionCmd.INFO_ALL_RSP), rsp[SessionCmd.INFO_ALL_RSP])


async def end_all_sessions(client: SessionClient) -> Tuple[bool, int]:
  # when tkn is 0, _send_query() will not set the 'tkn'
  rsp = await client._send_query(SessionCmd.END_ALL_REQ, {SessionCmd.END_ALL_REQ:{}})
  return (client._is_rsp_valid(rsp, SessionCmd.END_ALL_RSP), rsp[SessionCmd.END_ALL_RSP]['cnt'])
  



