from ndb.commands import StValues, Fields
from ndb.common import ResponseError
from ndb.connection import Connection
from ndb.logging import logger
from typing import Tuple, List

import logging


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


  async def open(self, uri: str) -> None:
    if uri == '':
      raise ValueError('URI empty')
        
    await self.ws.start(uri)
    logger.debug('Client connected')
    
    
  async def close(self):
    await self.ws.close()
 

  async def sendCmd(self, cmdReq: str, cmdRsp: str, body: dict, stSuccess = StValues.ST_SUCCESS, checkStatus = True):
    return await self._sendCmd(cmdReq, cmdRsp, body, stSuccess, checkStatus)
  

  async def sendCmd2(self, data:bytearray) -> bytes:
    return await self.ws.query2(data)
  

  async def _sendCmd(self, cmdReq: str, cmdRsp: str, body: dict, stSuccess = StValues.ST_SUCCESS, checkStatus = True):
    req = {cmdReq : body}
    rsp = await self.ws.query(req)
    if checkStatus:
      self._raise_if_invalid(rsp, cmdRsp, stSuccess)
    return rsp
  

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