from ndb.client import FieldValues, Fields, KvCmd
from ndb.kvclient import KvClient
from typing import Tuple, List



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
  SAVE_REQ      = 'SH_SAVE'
  SAVE_RSP      = 'SH_SAVE_RSP'
  LOAD_REQ      = 'SH_LOAD'
  LOAD_RSP      = 'SH_LOAD_RSP'



"""Stores the session token session.
"""
class Session:
  def __init__(self, tkn: int):
    self.tkn = tkn

  @property
  def isValid(self) -> bool:
    return self.tkn != 0



"""A client for when NemesisDB has sessions enabled.
Similar to KvClient but key value functions require a token, and
session specific functions are supplied.
"""
class SessionClient:
  def __init__(self):
    #super().__init__()
    self.client = KvClient()
    
  
  async def listen(self, uri: str):
    return await self.client.listen(uri)
  

  async def close(self):
    await self.client.close()


  async def set(self, keys: dict, tkn: int) -> bool:
    return await self.client.set(keys, tkn)
  

  async def add(self, keys: dict, tkn: int) -> bool:
    return await self.client.add(keys, tkn)


  async def get(self, keys: tuple, tkn: int) -> Tuple[bool, dict]:
    return await self.client.get(keys, tkn)
  

  async def rmv(self, keys: tuple, tkn: int) -> dict:
    return await self.client.rmv(keys, tkn)


  async def count(self, tkn: int) -> tuple:
    return await self.client.count(tkn)


  async def contains(self, keys: tuple, tkn: int) -> tuple:
    return await self.client.contains(keys, tkn)

  
  async def keys(self, tkn: int) -> tuple:
    return await self.client.keys(tkn)


  async def clear(self, tkn: int) -> tuple:
    return await self.client.keys(tkn)
    

  async def clear_set(self, keys: dict, tkn: int) -> tuple:
    return await self.client.clear_set(keys, tkn)


  """Create a new session, with optional expiry settings.
  expirySeconds - after this duration (seconds), the session expires. Default 0 - never expires.
  deleteSessionOnExpire - when True, the sessions is deleted. When false, the session is not deleted. 

  When a session expires, the keys are always deleted, but deleteSessionOnExpire controls if the 
  actual session is also deleted.
  """
  async def create_session(self, durationSeconds = 0, deleteSessionOnExpire = False) -> Session:
    # TODO add SH_NEW_SHARED command 
    q = {SessionCmd.NEW_REQ:{}}

    if durationSeconds < 0:
      raise ValueError('expirySeconds must be >= 0')
    
    if durationSeconds > 0:
      q[SessionCmd.NEW_REQ]['expiry'] = {'duration':durationSeconds, 'deleteSession':deleteSessionOnExpire}
      
    rsp = await self.client._send_query(SessionCmd.NEW_REQ, q)
    if self.client._is_rsp_valid(rsp, SessionCmd.NEW_RSP):
      return Session(rsp[SessionCmd.NEW_RSP]['tkn'])
    else:
      return Session(0)


  async def end_session(self, tkn: int):
    rsp = await self.client._send_session_query(SessionCmd.END_REQ, {SessionCmd.END_REQ:{}}, tkn)
    return self.client._is_rsp_valid(rsp, SessionCmd.END_RSP)


  async def end_all_sessions(self) -> Tuple[bool, int]:
    rsp = await self.client._send_query(SessionCmd.END_ALL_REQ, {SessionCmd.END_ALL_REQ:{}})
    return (self.client._is_rsp_valid(rsp, SessionCmd.END_ALL_RSP), rsp[SessionCmd.END_ALL_RSP]['cnt'])


  async def session_exists(self, tkns: List[int]) -> Tuple[bool, List]:
    # when tkn is 0, _send_query() will not set the 'tkn'
    rsp = await self.client._send_query(SessionCmd.EXISTS_REQ, {SessionCmd.EXISTS_REQ:{'tkns':tkns}})
    return (self.client._is_rsp_valid(rsp, SessionCmd.EXISTS_RSP), rsp[SessionCmd.EXISTS_RSP]['exist'])


  async def session_info(self, tkn: int) -> dict:
    rsp = await self.client._send_session_query(SessionCmd.INFO_REQ, {SessionCmd.INFO_REQ:{}}, tkn)
    return (self.client._is_rsp_valid(rsp, SessionCmd.INFO_RSP), rsp[SessionCmd.INFO_RSP])


  async def session_info_all(self) -> dict:
    # use _send_query() here because we don't set tkn
    rsp = await self.client._send_query(SessionCmd.INFO_ALL_REQ, {SessionCmd.INFO_ALL_REQ:{}})
    return (self.client._is_rsp_valid(rsp, SessionCmd.INFO_ALL_RSP), rsp[SessionCmd.INFO_ALL_RSP])

    
  async def save_session(self, name: str, tkn: int) -> bool:
    # use _send_query() here because we don't set tkn
    q = {SessionCmd.SAVE_REQ:{'name':name, 'tkns':[tkn]}}
    rsp = await self.client._send_query(SessionCmd.SAVE_REQ, q)
    return self.client._is_rsp_valid(rsp, SessionCmd.SAVE_RSP, FieldValues.ST_SAVE_COMPLETE)


  """Save all sessions or specific sessions.
  name - dataset name
  tkns - if empty saves all sessions, otherwise only sessions whose token is in 'tkns'
  """
  async def save_sessions(self, name: str, tkns = list()) -> bool:
    q = {SessionCmd.SAVE_REQ:{'name':name}}
    
    if len(tkns) > 0:
      q[SessionCmd.SAVE_REQ]['tkns'] = tkns

    rsp = await self.client._send_query(SessionCmd.SAVE_REQ, q)
    return self.client._is_rsp_valid(rsp, SessionCmd.SAVE_RSP, FieldValues.ST_SAVE_COMPLETE)


  async def load_session(self, name: str) -> Tuple[bool, dict]:
    q = {SessionCmd.LOAD_REQ:{'name':name}}
    rsp = await self.client._send_query(SessionCmd.LOAD_REQ, q)
    
    if self.client._is_rsp_valid(rsp, SessionCmd.LOAD_RSP, FieldValues.ST_LOAD_COMPLETE):
      return (True, rsp[SessionCmd.LOAD_RSP])
    else:
      return (False, dict())

