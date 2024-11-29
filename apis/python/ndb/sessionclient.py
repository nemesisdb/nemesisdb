from ndb.client import Client, StValues
from ndb.logging import logger
from typing import Tuple, List



class ShCmd:
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
  #
  SET_REQ       = 'SH_SET'
  SET_RSP       = 'SH_SET_RSP'
  ADD_REQ       = 'SH_ADD'
  ADD_RSP       = 'SH_ADD_RSP'
  GET_REQ       = 'SH_GET'
  GET_RSP       = 'SH_GET_RSP'
  RMV_REQ       = 'SH_RMV'
  RMV_RSP       = 'SH_RMV_RSP'
  COUNT_REQ     = 'SH_COUNT'
  COUNT_RSP     = 'SH_COUNT_RSP'
  CONTAINS_REQ  = 'SH_CONTAINS'
  CONTAINS_RSP  = 'SH_CONTAINS_RSP'
  CLEAR_REQ     = 'SH_CLEAR'
  CLEAR_RSP     = 'SH_CLEAR_RSP'
  CLEAR_SET_REQ = 'SH_CLEAR_SET'
  CLEAR_SET_RSP = 'SH_CLEAR_SET_RSP'
  KEYS_REQ      = 'SH_KEYS'
  KEYS_RSP      = 'SH_KEYS_RSP'




class Session:
  """Stores the session token session.
  """
  def __init__(self, tkn: int):
    self.tkn = tkn

  @property
  def isValid(self) -> bool:
    return self.tkn != 0




class SessionClient(Client):
  """A client for when NemesisDB has sessions enabled.
  Similar to KvClient but key value functions require a token, and
  session specific functions are supplied.
  """
  def __init__(self, debug = False):
    super().__init__(debug)
    
  
  async def set(self, keys: dict, tkn: int) -> bool:
    return await self._sendTknCmd(ShCmd.SET_REQ, ShCmd.SET_RSP, {'keys':keys}, tkn)
  

  async def add(self, keys: dict, tkn: int) -> bool:
    return await self._sendTknCmd(ShCmd.ADD_REQ, ShCmd.ADD_RSP, {'keys':keys}, tkn)


  async def get(self, keys: tuple, tkn: int) -> Tuple[bool, dict]:
    ok, rsp = await self._sendTknCmd(ShCmd.GET_REQ, ShCmd.GET_RSP, {'keys':keys}, tkn)
    return (ok, rsp[ShCmd.GET_RSP]['keys'] if ok else dict())


  async def rmv(self, keys: tuple, tkn: int) -> bool:
    ok, _ = await self._sendTknCmd(ShCmd.RMV_REQ, ShCmd.RMV_RSP, {'keys':keys}, tkn)
    return ok


  async def count(self, tkn: int) -> tuple:
    ok, rsp = await self._sendTknCmd(ShCmd.COUNT_REQ, ShCmd.COUNT_RSP, {}, tkn)
    return (ok, rsp[ShCmd.COUNT_RSP]['cnt'] if ok else 0)


  async def contains(self, keys: tuple, tkn: int) -> Tuple[bool, List]:
    ok, rsp = await self._sendTknCmd(ShCmd.CONTAINS_REQ, ShCmd.CONTAINS_RSP, {'keys':keys}, tkn)
    return (ok, rsp[ShCmd.CONTAINS_RSP]['contains'] if ok else [])

  
  async def keys(self, tkn: int) -> tuple:
    ok, rsp = await self._sendTknCmd(ShCmd.KEYS_REQ, ShCmd.KEYS_RSP, {}, tkn)
    return (ok, rsp[ShCmd.KEYS_RSP]['keys'] if ok else [])
  

  async def clear(self, tkn: int) -> Tuple[bool, int]:
    ok, rsp = await self._sendTknCmd(ShCmd.CLEAR_REQ, ShCmd.CLEAR_RSP, {}, tkn)
    return (ok, rsp[ShCmd.CLEAR_RSP]['cnt'] if ok else 0)
        

  async def clear_set(self, keys: dict, tkn: int) -> Tuple[bool, int]:
    ok, rsp = await self._sendTknCmd(ShCmd.CLEAR_SET_REQ, ShCmd.CLEAR_SET_RSP, {'keys':keys}, tkn)
    return (ok, rsp[ShCmd.CLEAR_SET_RSP]['cnt'] if ok else 0)


  
  async def create_session(self, durationSeconds = 0, deleteSessionOnExpire = False) -> Session:
    """Create a new session, with optional expiry settings.

    expirySeconds - after this duration (seconds), the session expires. Default 0 - never expires.
    deleteSessionOnExpire - when True, the sessions is deleted. When false, the session is not deleted. 

    When a session expires, the keys are always deleted, but deleteSessionOnExpire controls if the 
    actual session is also deleted.
    """
    body = dict()

    if durationSeconds < 0:
      raise ValueError('expirySeconds must be >= 0')
    
    if durationSeconds > 0:
      body['expiry'] = {'duration':durationSeconds, 'deleteSession':deleteSessionOnExpire}
      
    ok, rsp = await self.sendCmd(ShCmd.NEW_REQ, ShCmd.NEW_RSP, body)
    if ok:
      token = rsp[ShCmd.NEW_RSP]['tkn']
      logger.debug(f'Created session: {token}')
      return Session(token)
    else:
      return Session(0)


  async def end_session(self, tkn: int):
    ok, _ = await self._sendTknCmd(ShCmd.END_REQ, ShCmd.END_RSP, {}, tkn)
    return ok


  async def end_all_sessions(self) -> Tuple[bool, int]:
    ok, rsp = await self.sendCmd(ShCmd.END_ALL_REQ, ShCmd.END_ALL_RSP, {})
    return (ok, rsp[ShCmd.END_ALL_RSP]['cnt'] if ok else 0)


  async def session_exists(self, tkns: List[int]) -> Tuple[bool, List]:
    ok, rsp = await self.sendCmd(ShCmd.EXISTS_REQ, ShCmd.EXISTS_RSP, {'tkns':tkns})
    return (ok, rsp[ShCmd.EXISTS_RSP]['exist'] if ok else [])
    

  async def session_info(self, tkn: int) -> Tuple[bool,dict]:
    ok, rsp = await self._sendTknCmd(ShCmd.INFO_REQ, ShCmd.INFO_RSP, {}, tkn)
    if ok:
      info = dict(rsp[ShCmd.INFO_RSP])
      info.pop('st')
      return (True, info)
    else:
      return (False, dict())
    
    
  async def session_info_all(self) -> Tuple[bool,dict]:
    ok, rsp = await self.sendCmd(ShCmd.INFO_ALL_REQ, ShCmd.INFO_ALL_RSP, {})
    if ok:
      info = dict(rsp[ShCmd.INFO_ALL_RSP])
      info.pop('st')
      return (True, info)
    else:
      return (False, dict())
    
  
  async def session_save(self, name: str, tkns = list()) -> bool:
    """Save all sessions or specific sessions.
    name - dataset name
    tkns - if empty, saves all sessions, otherwise only sessions whose token is in 'tkns'
    """
    if name == '':
      raise ValueError('Name cannot be empty')

    body = {'name':name}
    
    if len(tkns) > 0:
      body['tkns'] = tkns

    ok, _ = await self.sendCmd(ShCmd.SAVE_REQ, ShCmd.SAVE_RSP, body, StValues.ST_SAVE_COMPLETE)
    return ok


  async def session_load(self, name: str) -> Tuple[bool, dict]:
    """Loads session data from persistance.
    name - the same name as used with session_save().
    """
    ok, rsp = await self.sendCmd(ShCmd.LOAD_REQ, ShCmd.LOAD_RSP, {'name':name}, StValues.ST_LOAD_COMPLETE)
    if ok:
      info = dict(rsp[ShCmd.LOAD_RSP])
      info.pop('st')
      return (True, info)
    else:
      return (False, dict())
      

  async def _sendTknCmd(self, cmdReq: str, cmdRsp: str, body: dict, tkn: int):
    body['tkn'] = tkn   
    return await self.sendCmd(cmdReq, cmdRsp, body)
  

