from ndb.commands import StValues, Fields


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


def raise_if_invalid(rsp: dict, cmdRsp: str, expected = StValues.ST_SUCCESS):
  if rsp[cmdRsp][Fields.STATUS] != expected:
    raise ResponseError(rsp[cmdRsp])


def raise_if_empty (value: str):
  raise_if(value, 'empty', lambda v: v == '')
  #if value == '':
    # raise ValueError('value empty')


def raise_if (value: str, msg: str, f):
  "Calls f(value), and if f() return True, prints 'value is {msg}'"
  if f(value):
    raise ValueError(f'value is {msg}')