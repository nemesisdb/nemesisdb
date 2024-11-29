from abc import ABC
from enum import Enum
from ndb.connection import Connection
from ndb.logging import logger
from typing import Tuple
from asyncio import CancelledError
import logging


class StValues:
  """
  There are more but we only require:
    ST_SUCCESS - command success
    ST_SAVE_COMPLETE - SH_SAVE or KV_SAVE success, data persisted
    ST_SAVE_ERROR - SH_SAVE or KV_SAVE fail
    ST_LOAD_COMPLETE - SH_LOAD or KV_LOAD success, data available
  """
  ST_SUCCESS = 1
  ST_SAVE_COMPLETE = 121
  ST_SAVE_ERROR = 123
  ST_LOAD_COMPLETE = 141


class Fields:
  STATUS    = 'st'


class SvCmd:
  INFO_REQ    = 'SV_INFO'
  INFO_RSP    = 'SV_INFO_RSP'


class NdbException(Exception):
  pass


class ConnectError(NdbException):
   def __init__(self, msg='Connection failed'):
    super().__init__(msg)


class ResponseError(NdbException):
  def __init__(self, rsp):
    super().__init__('Response failed')
    self.rsp = rsp



  
class Client(ABC):
  """Inherited by KvClient and SessionClient to query the database.

  Contains functions common to session and kv clients. 
  """

  def __init__(self, debug = False):
    self.uri = ''
    self.api = Connection()
    if debug:
      logger.setLevel(logging.DEBUG)
      logger.addHandler(logging.StreamHandler())
      
  
  async def open(self, uri: str) -> bool:
    if uri == '':
      raise ValueError('URI empty')
        
    connected = await self.api.start(uri)
    logger.debug('Client: connected' if connected else 'Client: failed to connect')
    return connected


  async def _sendCmd(self, cmdReq: str, cmdRsp: str, body: dict, stSuccess = StValues.ST_SUCCESS):
    req = {cmdReq : body}
    rsp = await self.api.query(req)
    self._raise_if_invalid(rsp, cmdRsp, stSuccess)
    return rsp
    

  # server_info is independent from kv and sh
  async def server_info(self) -> dict:
    rsp = await self._sendCmd(SvCmd.INFO_REQ, SvCmd.INFO_RSP, {})
    info = dict(rsp.get(SvCmd.INFO_RSP))
    info.pop('st')
    return info


  async def close(self):
    await self.api.close()
 

  def _raise_if_invalid(self, rsp: dict, cmdRsp: str, expected = StValues.ST_SUCCESS):
    if rsp[cmdRsp][Fields.STATUS] != expected:
      raise ResponseError(rsp)
    
    logger.debug(cmdRsp + ' ' + ('Response Ok'))
    
