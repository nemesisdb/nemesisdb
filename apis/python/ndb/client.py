from ndb.commands import StValues, Fields
from ndb.common import ResponseError, raise_if_invalid, raise_if_fail
from ndb.connection import Connection
from ndb.logging import logger
from typing import Tuple, List



class NdbClient:
  def __init__(self):
    self.uri = ''
    self.ws = Connection()
    
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
    rsp = await self.ws.query2(data)
    raise_if_fail(rsp)
    return rsp
  

  async def _sendCmd(self, cmdReq: str, cmdRsp: str, body: dict, stSuccess = StValues.ST_SUCCESS, checkStatus = True):
    req = {cmdReq : body}
    rsp = await self.ws.query(req)
    if checkStatus:
      raise_if_invalid(rsp, cmdRsp, stSuccess)
    return rsp
  

  # def _raise_if_invalid(self, rsp: dict, cmdRsp: str, expected = StValues.ST_SUCCESS):
  #   if cmdRsp in rsp:
  #     if rsp[cmdRsp][Fields.STATUS] != expected:
  #       raise ResponseError(rsp[cmdRsp])
  #   elif 'ERR' in rsp:  # 'ERR' returned if the server cannot return the original request
  #     raise ResponseError(rsp['ERR'])
  #   else:
  #     logger.debug(cmdRsp + ' ' + ('Response Ok'))


  # def _raise_if_empty (self, value: str):
  #   self._raise_if(value, 'empty', lambda v: v == '')
  #   #if value == '':
  #    # raise ValueError('value empty')
  

  # def _raise_if (self, value: str, msg: str, f):
  #   if f(value):
  #     raise ValueError(f'value is {msg}')