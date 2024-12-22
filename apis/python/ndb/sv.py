from ndb.commands import (Fields, SvCmds)
from ndb.client import NdbClient
from typing import List, Any


class SV:
  "Server Info"

  def __init__(self, client: NdbClient):
    self.client = client
    self.cmds = SvCmds()


  async def info(self) -> dict:
    rsp = await self.client.sendCmd(self.cmds.INFO_REQ, self.cmds.INFO_RSP, {})
    info = dict(rsp.get(self.cmds.INFO_RSP))
    info.pop(Fields.STATUS)
    return info
  