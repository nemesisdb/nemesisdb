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
  """A client to manage sessions and keys in a session.
  create_session() creates a session, returning a Session which contains the token.
  The Session.tkn is used in subsequent commands.
  """
  def __init__(self, debug = False):
    super().__init__(debug)
    
  
  async def set(self, keys: dict, tkn: int):
    await self._sendTknCmd(ShCmd.SET_REQ, ShCmd.SET_RSP, {'keys':keys}, tkn)
  

  async def add(self, keys: dict, tkn: int):
    await self._sendTknCmd(ShCmd.ADD_REQ, ShCmd.ADD_RSP, {'keys':keys}, tkn)


  async def get(self, keys: tuple, tkn: int) -> dict:
    rsp = await self._sendTknCmd(ShCmd.GET_REQ, ShCmd.GET_RSP, {'keys':keys}, tkn)
    return rsp[ShCmd.GET_RSP]['keys']


  async def rmv(self, keys: tuple, tkn: int) -> None:
    await self._sendTknCmd(ShCmd.RMV_REQ, ShCmd.RMV_RSP, {'keys':keys}, tkn)


  async def count(self, tkn: int) -> int:
    rsp = await self._sendTknCmd(ShCmd.COUNT_REQ, ShCmd.COUNT_RSP, {}, tkn)
    return rsp[ShCmd.COUNT_RSP]['cnt'] 


  async def contains(self, keys: tuple, tkn: int) -> Tuple[bool, List]:
    rsp = await self._sendTknCmd(ShCmd.CONTAINS_REQ, ShCmd.CONTAINS_RSP, {'keys':keys}, tkn)
    return rsp[ShCmd.CONTAINS_RSP]['contains']

  
  async def keys(self, tkn: int) -> List:
    ok, rsp = await self._sendTknCmd(ShCmd.KEYS_REQ, ShCmd.KEYS_RSP, {}, tkn)
    return rsp[ShCmd.KEYS_RSP]['keys'] 
  

  async def clear(self, tkn: int) -> int:
    """Clear all keys in the session. To delete all sessions, use end_all_sessions()."""
    rsp = await self._sendTknCmd(ShCmd.CLEAR_REQ, ShCmd.CLEAR_RSP, {}, tkn)
    return rsp[ShCmd.CLEAR_RSP]['cnt']
        

  async def clear_set(self, keys: dict, tkn: int) -> int:
    """Clear keys in the session then set new keys."""
    rsp = await self._sendTknCmd(ShCmd.CLEAR_SET_REQ, ShCmd.CLEAR_SET_RSP, {'keys':keys}, tkn)
    return rsp[ShCmd.CLEAR_SET_RSP]['cnt']

  
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
      
    rsp = await self._sendCmd(ShCmd.NEW_REQ, ShCmd.NEW_RSP, body)
    token = rsp[ShCmd.NEW_RSP]['tkn']
    logger.debug(f'Created session: {token}')
    return Session(token)


  async def end_session(self, tkn: int):
    await self._sendTknCmd(ShCmd.END_REQ, ShCmd.END_RSP, {}, tkn)


  async def end_all_sessions(self) -> int:
    rsp = await self._sendCmd(ShCmd.END_ALL_REQ, ShCmd.END_ALL_RSP, {})
    return rsp[ShCmd.END_ALL_RSP]['cnt']


  async def session_exists(self, tkns: List[int]) ->  List:
    rsp = await self._sendCmd(ShCmd.EXISTS_REQ, ShCmd.EXISTS_RSP, {'tkns':tkns})
    return rsp[ShCmd.EXISTS_RSP]['exist'] 
    

  async def session_info(self, tkn: int) -> dict:
    rsp = await self._sendTknCmd(ShCmd.INFO_REQ, ShCmd.INFO_RSP, {}, tkn)
    info = dict(rsp[ShCmd.INFO_RSP])
    info.pop('st')
    return info
    
    
  async def session_info_all(self) -> dict:
    rsp = await self._sendCmd(ShCmd.INFO_ALL_REQ, ShCmd.INFO_ALL_RSP, {})
    info = dict(rsp[ShCmd.INFO_ALL_RSP])
    info.pop('st')
    return info
    
  
  async def save(self, name: str, tkns = list()):
    """Save all sessions or specific sessions.
    name - dataset name
    tkns - if empty, saves all sessions, otherwise only sessions whose token is in 'tkns'
    """
    if name == '':
      raise ValueError('Name cannot be empty')

    body = {'name':name}
    
    if len(tkns) > 0:
      body['tkns'] = tkns

    await self._sendCmd(ShCmd.SAVE_REQ, ShCmd.SAVE_RSP, body, StValues.ST_SAVE_COMPLETE)


  async def load(self, name: str) -> dict:
    """Loads session data from persistance.
    name - the same name as used with session_save().
    """
    rsp = await self._sendCmd(ShCmd.LOAD_REQ, ShCmd.LOAD_RSP, {'name':name}, StValues.ST_LOAD_COMPLETE)
    info = dict(rsp[ShCmd.LOAD_RSP])
    info.pop('st')
    return info
      

  async def _sendTknCmd(self, cmdReq: str, cmdRsp: str, body: dict, tkn: int):
    body['tkn'] = tkn   
    return await self._sendCmd(cmdReq, cmdRsp, body)
  

