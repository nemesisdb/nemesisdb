import sys
from unittest import IsolatedAsyncioTestCase

sys.path.append('../apis/python')
from ndb.kvclient import KvClient


class NDBTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = KvClient()
    
    connected = await self.client.open('ws://127.0.0.1:1987')
    self.assertTrue(connected, 'Connection failed')
    
    # clear before each test
    (valid, _) = await self.client.clear()
    self.assertTrue(valid)