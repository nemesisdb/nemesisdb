from ndb.commands import StValues, Fields, SvCmd
from ndb.connection import Connection
from ndb.logging import logger
from typing import Tuple, List

import logging

## MOVE to common
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


## MOVE to sessions.py
class Session:
  """Stores the session token session.
  """
  def __init__(self, tkn: int):
    self.tkn = tkn

  @property
  def tknValid(self) -> bool:
    return self.tkn > 0



class NdbClient:
  def __init__(self, debug = False):
    self.uri = ''
    self.ws = Connection()
    if debug:
      logger.setLevel(logging.DEBUG)
      logger.addHandler(logging.StreamHandler())


  # def __str__(self):
  #   if self.ws.opened != None and self.ws.opened:
  #     connected = 'Connected' 
  #   else:
  #     connected = 'Disconnected' 

  #   return f'NDB - {self.uri} - {connected}'


  async def open(self, uri: str) -> bool:
    if uri == '':
      raise ValueError('URI empty')
        
    await self.ws.start(uri)
    logger.debug('Client connected')
    
    
  async def close(self):
    await self.ws.close()
 

  ## SV
  async def sv_info(self) -> dict:
    rsp = await self._sendCmd(SvCmd.INFO_REQ, SvCmd.INFO_RSP, {})
    info = dict(rsp.get(SvCmd.INFO_RSP))
    info.pop('st')
    return info
  

  #region KV
  # async def kv_set(self, keys: dict) -> None:
  #   await self._sendCmd(KvCmd.SET_REQ, KvCmd.SET_RSP, {'keys':keys})
  

  # async def kv_add(self, keys: dict) -> None:
  #   await self._sendCmd(KvCmd.ADD_REQ, KvCmd.ADD_RSP, {'keys':keys})


  # async def kv_get(self, keys = tuple(), key=None):
  #   if key != None and len(keys):
  #     raise ValueError('Both keys and key are set')
  #   elif key != None:
  #     return await self._kv_get_single(key)
  #   else:
  #     return await self._kv_get_multiple(keys)

  
  # async def _kv_get_single(self, key: str):
  #   rsp = await self._sendCmd(KvCmd.GET_REQ, KvCmd.GET_RSP, {'keys':[key]})
  #   if key in rsp[KvCmd.GET_RSP]['keys']:
  #     return rsp[KvCmd.GET_RSP]['keys'][key]
  #   else:
  #     return None
  

  # async def _kv_get_multiple(self, keys: tuple) -> dict:
  #   rsp = await self._sendCmd(KvCmd.GET_REQ, KvCmd.GET_RSP, {'keys':keys})
  #   return rsp[KvCmd.GET_RSP]['keys']
  

  # async def kv_rmv(self, keys: tuple) -> None:
  #   if len(keys) == 0:
  #     raise ValueError('Keys empty')
  #   await self._sendCmd(KvCmd.RMV_REQ, KvCmd.RMV_RSP, {'keys':keys})


  # async def kv_count(self) -> int:
  #   rsp = await self._sendCmd(KvCmd.COUNT_REQ, KvCmd.COUNT_RSP, {})
  #   return rsp[KvCmd.COUNT_RSP]['cnt']


  # async def kv_contains(self, keys: tuple) -> List[str]:
  #   rsp = await self._sendCmd(KvCmd.CONTAINS_REQ, KvCmd.CONTAINS_RSP, {'keys':keys})
  #   return rsp[KvCmd.CONTAINS_RSP]['contains']

  
  # async def kv_keys(self) -> List[str]:
  #   rsp = await self._sendCmd(KvCmd.KEYS_REQ, KvCmd.KEYS_RSP, {})
  #   return rsp[KvCmd.KEYS_RSP]['keys']
  

  # async def kv_clear(self) -> int:
  #   rsp = await self._sendCmd(KvCmd.CLEAR_REQ, KvCmd.CLEAR_RSP, {})
  #   return rsp[KvCmd.CLEAR_RSP]['cnt']
        

  # async def kv_clear_set(self, keys: dict) -> int:
  #   rsp = await self._sendCmd(KvCmd.CLEAR_SET_REQ, KvCmd.CLEAR_SET_RSP, {'keys':keys})
  #   return rsp[KvCmd.CLEAR_SET_RSP]['cnt'] 


  # async def kv_save(self, name: str) -> None:
  #   if len(name) == 0:
  #     raise ValueError('Name empty')
  #   await self._sendCmd(KvCmd.SAVE_REQ, KvCmd.SAVE_RSP, {'name':name}, StValues.ST_SAVE_COMPLETE)
    

  # async def kv_load(self, name: str) -> int:
  #   if len(name) == 0:
  #     raise ValueError('Name empty')
  #   rsp = await self._sendCmd(KvCmd.LOAD_REQ, KvCmd.LOAD_RSP, {'name':name}, StValues.ST_LOAD_COMPLETE)
  #   return rsp[KvCmd.LOAD_RSP]['keys']

  #endregion

  
  async def _sendCmd(self, cmdReq: str, cmdRsp: str, body: dict, stSuccess = StValues.ST_SUCCESS, checkStatus = True):
    req = {cmdReq : body}
    rsp = await self.ws.query(req)
    if checkStatus:
      self._raise_if_invalid(rsp, cmdRsp, stSuccess)
    return rsp
  

  async def sendCmd(self, cmdReq: str, cmdRsp: str, body: dict, stSuccess = StValues.ST_SUCCESS, checkStatus = True):
    return await self._sendCmd(cmdReq, cmdRsp, body, stSuccess, checkStatus)
  

  async def _sendTknCmd(self, cmdReq: str, cmdRsp: str, body: dict, tkn: int):
    body[Fields.TOKEN] = tkn   
    return await self._sendCmd(cmdReq, cmdRsp, body)
  

  def _raise_if_invalid(self, rsp: dict, cmdRsp: str, expected = StValues.ST_SUCCESS):
    if cmdRsp in rsp:
      if rsp[cmdRsp][Fields.STATUS] != expected:
        raise ResponseError(rsp[cmdRsp])
    elif 'ERR' in rsp:  # 'ERR' returned if the server cannot return the original request
      raise ResponseError(rsp['ERR'])
    else:
      logger.debug(cmdRsp + ' ' + ('Response Ok'))


  def _raise_if_empty (self, value: str):
    self._raise_if(value, 'empty', lambda v: v == '')
    #if value == '':
     # raise ValueError('value empty')
  

  def _raise_if (self, value: str, msg: str, f):
    if f(value):
      raise ValueError(f'value is {msg}')