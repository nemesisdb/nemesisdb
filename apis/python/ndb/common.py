from ndb.commands import StValues, Fields
from ndb.fbs.ndb.response import Response, Status
import flatbuffers
import flatbuffers.flexbuffers
import builtins


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


class ResponseError2(NdbException):
  def __init__(self, status: Status):
    super().__init__(f'Response Error: {status}')
    self.status = status



def CreateKvArray(kv: dict) -> bytearray:
  # NOTE MapFromElements() does not differentiate between Int and UInt
  # so do serialising manually here
  b = flatbuffers.flexbuffers.Builder()

  def getValueTypeAndAdd(v):
    match type(v):
      case builtins.int:
        return b.Int if v < 0 else b.UInt
      
      case builtins.str:
        return b.String

      case builtins.float:
        return b.Float

      case builtins.bool:
        return b.Bool
    
      case _:
        return None

  with b.Map():
    for k,v in kv.items():
      f = getValueTypeAndAdd(v)
      if f:
        b.Key(k)
        f(v)

  return b.Finish()


def raise_if_fail(rsp: bytes):
  response = Response.Response.GetRootAs(rsp)
  if response.Status() is not Status.Status.Ok:
    raise ResponseError2(response.Status())
  

def raise_if_invalid(rsp: dict, cmdRsp: str, expected = StValues.ST_SUCCESS):
  if rsp[cmdRsp][Fields.STATUS] != expected:
    raise ResponseError(rsp[cmdRsp])


def raise_if_empty (value: str):
  if value == '':
    raise ValueError('value empty')


def raise_if_lt (value: int, maxValue:int, msg: str):
  if value < maxValue:
    raise ValueError(msg)


def raise_if_equal (val1, val2, msg:str):
  if val1 == val2:  
    raise ValueError(msg)
  

def raise_if (condition, msg:str, errorType = ValueError):
  if condition:
    raise errorType(msg)


def raise_if_not (condition, msg:str, errorType = ValueError):
  if not raise_if(condition, msg, errorType):
    raise errorType(msg)
  

