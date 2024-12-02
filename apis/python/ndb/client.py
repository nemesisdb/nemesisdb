from ndb.commands import StValues, Fields, SvCmd, KvCmd, ShCmd
from ndb.connection import Connection
from ndb.logging import logger
from typing import Tuple, List

import logging


class NdbException(Exception):
  def __init__(self, msg):
    super().__init__(msg)


class ResponseError(NdbException):
  def __init__(self, body):
    if Fields.STATUS in body:
      msg = f'Response Failed with status: {body[Fields.STATUS]}'
    else:
      msg = 'Response failed'

    super().__init__(msg)
    self.rsp = body


class Session:
  """Stores the session token session.
  """
  def __init__(self, tkn: int):
    self.tkn = tkn

  @property
  def isValid(self) -> bool:
    return self.tkn != 0



class NdbClient:
  def __init__(self, debug = False):
    self.uri = ''
    self.ws = Connection()
    if debug:
      logger.setLevel(logging.DEBUG)
      logger.addHandler(logging.StreamHandler())


  async def open(self, uri: str) -> bool:
    if uri == '':
      raise ValueError('URI empty')
        
    connected = await self.ws.start(uri)
    logger.debug('Client: connected' if connected else 'Client: failed to connect')
    return connected

    
  async def close(self):
    await self.ws.close()
 

  ## SV
  async def sv_info(self) -> dict:
    rsp = await self._sendCmd(SvCmd.INFO_REQ, SvCmd.INFO_RSP, {})
    info = dict(rsp.get(SvCmd.INFO_RSP))
    info.pop('st')
    return info
  

  ## KV
  async def kv_set(self, keys: dict):
    await self._sendCmd(KvCmd.SET_REQ, KvCmd.SET_RSP, {'keys':keys})
  

  async def kv_add(self, keys: dict) -> bool:
    await self._sendCmd(KvCmd.ADD_REQ, KvCmd.ADD_RSP, {'keys':keys})


  async def kv_get(self, keys: tuple) -> dict:
    rsp = await self._sendCmd(KvCmd.GET_REQ, KvCmd.GET_RSP, {'keys':keys})
    return rsp[KvCmd.GET_RSP]['keys']


  async def kv_rmv(self, keys: tuple):
    await self._sendCmd(KvCmd.RMV_REQ, KvCmd.RMV_RSP, {'keys':keys})


  async def kv_count(self) -> int:
    rsp = await self._sendCmd(KvCmd.COUNT_REQ, KvCmd.COUNT_RSP, {})
    return rsp[KvCmd.COUNT_RSP]['cnt']


  async def kv_contains(self, keys: tuple) -> List:
    rsp = await self._sendCmd(KvCmd.CONTAINS_REQ, KvCmd.CONTAINS_RSP, {'keys':keys})
    return rsp[KvCmd.CONTAINS_RSP]['contains']

  
  async def kv_keys(self) -> int:
    rsp = await self._sendCmd(KvCmd.KEYS_REQ, KvCmd.KEYS_RSP, {})
    return rsp[KvCmd.KEYS_RSP]['keys']
  

  async def kv_clear(self) -> int:
    rsp = await self._sendCmd(KvCmd.CLEAR_REQ, KvCmd.CLEAR_RSP, {})
    return rsp[KvCmd.CLEAR_RSP]['cnt']
        

  async def kv_clear_set(self, keys: dict) -> int:
    rsp = await self._sendCmd(KvCmd.CLEAR_SET_REQ, KvCmd.CLEAR_SET_RSP, {'keys':keys})
    return rsp[KvCmd.CLEAR_SET_RSP]['cnt'] 


  async def kv_save(self, name: str):
    await self._sendCmd(KvCmd.SAVE_REQ, KvCmd.SAVE_RSP, {'name':name}, StValues.ST_SAVE_COMPLETE)
    

  async def kv_load(self, name: str) -> int:
    rsp = await self._sendCmd(KvCmd.LOAD_REQ, KvCmd.LOAD_RSP, {'name':name}, StValues.ST_LOAD_COMPLETE)
    return rsp[KvCmd.LOAD_RSP]['keys']


  ## SH
  async def sh_set(self, keys: dict, tkn: int):
    await self._sendTknCmd(ShCmd.SET_REQ, ShCmd.SET_RSP, {'keys':keys}, tkn)
  

  async def sh_add(self, keys: dict, tkn: int):
    await self._sendTknCmd(ShCmd.ADD_REQ, ShCmd.ADD_RSP, {'keys':keys}, tkn)


  async def sh_get(self, keys: tuple, tkn: int) -> dict:
    rsp = await self._sendTknCmd(ShCmd.GET_REQ, ShCmd.GET_RSP, {'keys':keys}, tkn)
    return rsp[ShCmd.GET_RSP]['keys']


  async def sh_rmv(self, keys: tuple, tkn: int) -> None:
    await self._sendTknCmd(ShCmd.RMV_REQ, ShCmd.RMV_RSP, {'keys':keys}, tkn)


  async def sh_count(self, tkn: int) -> int:
    rsp = await self._sendTknCmd(ShCmd.COUNT_REQ, ShCmd.COUNT_RSP, {}, tkn)
    return rsp[ShCmd.COUNT_RSP]['cnt'] 


  async def sh_contains(self, keys: tuple, tkn: int) -> List:
    rsp = await self._sendTknCmd(ShCmd.CONTAINS_REQ, ShCmd.CONTAINS_RSP, {'keys':keys}, tkn)
    return rsp[ShCmd.CONTAINS_RSP]['contains']

  
  async def sh_keys(self, tkn: int) -> List:
    rsp = await self._sendTknCmd(ShCmd.KEYS_REQ, ShCmd.KEYS_RSP, {}, tkn)
    return rsp[ShCmd.KEYS_RSP]['keys'] 
  

  async def sh_clear(self, tkn: int) -> int:
    """Clear all keys in the session. To delete all sessions, use end_all_sessions()."""
    rsp = await self._sendTknCmd(ShCmd.CLEAR_REQ, ShCmd.CLEAR_RSP, {}, tkn)
    return rsp[ShCmd.CLEAR_RSP]['cnt']
        

  async def sh_clear_set(self, keys: dict, tkn: int) -> int:
    """Clear keys in the session then set new keys."""
    rsp = await self._sendTknCmd(ShCmd.CLEAR_SET_REQ, ShCmd.CLEAR_SET_RSP, {'keys':keys}, tkn)
    return rsp[ShCmd.CLEAR_SET_RSP]['cnt']

  
  async def sh_create_session(self,  durationSeconds = 0,
                                  deleteSessionOnExpire = False,
                                  extendOnSetAdd = False,
                                  extendOnGet = False) -> Session:
    """Create a new session, with optional expiry settings.

    durationSeconds: After this duration (seconds), all keys in the session are deleted. Default 0 - never expires.

    if durationSeconds > 0
      deleteSessionOnExpire - when True, the session is deleted. When false, the session is not deleted. 
      extendOnSetAdd - if True, extend the expire time by durationSeconds on each set or add operation
      extendOnGet - if True, extend the expire time by durationSeconds on each get operation
    
    Returns: Session object containing the isValid property and the session token.
    """
    body = dict()
    
    if durationSeconds < 0:
      raise ValueError('durationSeconds must be < 0')
    elif durationSeconds == 0 and (deleteSessionOnExpire or extendOnSetAdd or extendOnGet):
      # having these set with duration of 0 does not cause a problem on the server, but implies
      # a misunderstanding or unintentional value
      raise ValueError('durationSeconds is 0 so deleteSessionOnExpire, extendOnSetAdd and extendOnGet have no affect')
    
    
    if durationSeconds > 0:
      body['expiry'] = {
                        'duration':durationSeconds,
                        'deleteSession':deleteSessionOnExpire,
                        'extendOnSetAdd':extendOnSetAdd,
                        'extendOnGet':extendOnGet
                       }
    

    rsp = await self._sendCmd(ShCmd.NEW_REQ, ShCmd.NEW_RSP, body)
    token = rsp[ShCmd.NEW_RSP]['tkn']
    logger.debug(f'Created session: {token}')
    return Session(token)


  async def sh_end(self, tkn: int):
    await self._sendTknCmd(ShCmd.END_REQ, ShCmd.END_RSP, {}, tkn)


  async def sh_end_all(self) -> int:
    rsp = await self._sendCmd(ShCmd.END_ALL_REQ, ShCmd.END_ALL_RSP, {})
    return rsp[ShCmd.END_ALL_RSP]['cnt']


  async def sh_session_exists(self, tkns: List[int]) ->  List:
    rsp = await self._sendCmd(ShCmd.EXISTS_REQ, ShCmd.EXISTS_RSP, {'tkns':tkns})
    return rsp[ShCmd.EXISTS_RSP]['exist'] 
    

  async def sh_info(self, tkn: int) -> dict:
    rsp = await self._sendTknCmd(ShCmd.INFO_REQ, ShCmd.INFO_RSP, {}, tkn)
    info = dict(rsp[ShCmd.INFO_RSP])
    info.pop(Fields.STATUS)
    return info
    
    
  async def sh_info_all(self) -> dict:
    rsp = await self._sendCmd(ShCmd.INFO_ALL_REQ, ShCmd.INFO_ALL_RSP, {})
    info = dict(rsp[ShCmd.INFO_ALL_RSP])
    info.pop(Fields.STATUS)
    return info
    
  
  async def sh_save(self, name: str, tkns = list()):
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


  async def sh_load(self, name: str) -> dict:
    """Loads session data from persistance.

    name - a name previously used with sh_save().
    """
    rsp = await self._sendCmd(ShCmd.LOAD_REQ, ShCmd.LOAD_RSP, {'name':name}, StValues.ST_LOAD_COMPLETE)
    info = dict(rsp[ShCmd.LOAD_RSP])
    info.pop('st')
    return info


  async def _sendCmd(self, cmdReq: str, cmdRsp: str, body: dict, stSuccess = StValues.ST_SUCCESS):
    req = {cmdReq : body}
    rsp = await self.ws.query(req)
    self._raise_if_invalid(rsp, cmdRsp, stSuccess)
    return rsp
  

  async def _sendTknCmd(self, cmdReq: str, cmdRsp: str, body: dict, tkn: int):
    body[Fields.TOKEN] = tkn   
    return await self._sendCmd(cmdReq, cmdRsp, body)
  

  def _raise_if_invalid(self, rsp: dict, cmdRsp: str, expected = StValues.ST_SUCCESS):
    if rsp[cmdRsp][Fields.STATUS] != expected:
      raise ResponseError(rsp[cmdRsp])
    
    logger.debug(cmdRsp + ' ' + ('Response Ok'))

  