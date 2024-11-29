from abc import ABC
from enum import Enum
from ndb.connection import Connection
from ndb.logging import logger
from typing import Tuple
from asyncio import CancelledError
import logging


class StValues:
  """
  There are more but we only check specificy for these.
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


class Client(ABC):
  """Used by KvClient and SessionClient to query the database.

  Provides a function per database command.

  Most functions have an optional 'tkn' parameter - this is only relevant when
  using the SessionClient. When sessions are disabled, leave the 'tkn' as default.

  Note: KV_SETQ and KV_ADDQ are not supported.
  """

  def __init__(self, debug = False):
    self.uri = ''
    self.listen_task = None
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


  async def sendCmd(self, cmdReq: str, cmdRsp: str, body: dict, stSuccess = StValues.ST_SUCCESS):
    req = {cmdReq : body}
    rsp = await self._send_query(cmdReq, req)
    return (self._is_rsp_valid(rsp, cmdRsp, stSuccess), rsp)
  
  
  async def sendTknCmd(self, cmdReq: str, cmdRsp: str, body: dict, tkn: int):
    body['tkn'] = tkn
    return await self.sendCmd(cmdReq, cmdRsp, body)
  

  async def server_info(self) -> Tuple[bool, dict]:
    rsp = await self._send_query(SvCmd.INFO_REQ, {SvCmd.INFO_REQ : {}})
    if self._is_rsp_valid(rsp, SvCmd.INFO_RSP):
      info = dict(rsp.get(SvCmd.INFO_RSP))
      info.pop('st')
      return (True, info)
    else:
      return (False, dict())


  async def close(self):
    await self.api.close()


  def _is_rsp_valid(self, rsp: dict, cmd: str, expected = StValues.ST_SUCCESS) -> bool:
    valid = rsp[cmd][Fields.STATUS] == expected
    logger.debug(cmd + ' ' + ('Response Ok' if valid else f'Response Fail: {str(rsp)}'))
    return valid

  
  async def _send_query(self, cmd: str, q: dict, tkn = 0):
    logger.debug('Send query: '+ str(q))
    if tkn != 0:
      return await self._send_session_query(cmd, q, tkn)
    else:
      return await self._send_kv_query(q)


  async def _send_kv_query(self, q: dict) -> dict:
    return await self.api.query(q)

  
  async def _send_session_query(self, cmd: str, q: dict, tkn: int) -> dict:
    q[cmd]['tkn'] = tkn
    return await self.api.query(q)
