from unittest import IsolatedAsyncioTestCase
from ndb.kvclient import KvClient
from ndb.sessionclient import SessionClient


class NDBTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = KvClient()
    
    connected = await self.client.open('ws://127.0.0.1:1987')
    self.assertTrue(connected, 'Connection failed')
    
    # clear before each test
    await self.client.clear()


class NDBSessionTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = SessionClient()
    
    connected = await self.client.open('ws://127.0.0.1:1987')
    self.assertTrue(connected, 'Connection failed')
    
    # clear before each test
    await self.client.end_all_sessions()