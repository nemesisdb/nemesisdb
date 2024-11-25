import sys
import os
from unittest import IsolatedAsyncioTestCase

sys.path.append(os.path.abspath('../../apis/python'))
from ndb.kvclient import KvClient
from ndb.sessionclient import SessionClient


class NDBTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = KvClient()
    
    connected = await self.client.open('ws://127.0.0.1:1987')
    self.assertTrue(connected, 'Connection failed')
    
    # clear before each test
    (valid, _) = await self.client.clear()
    self.assertTrue(valid)


class NDBSessionTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = SessionClient()
    
    connected = await self.client.open('ws://127.0.0.1:1987')
    self.assertTrue(connected, 'Connection failed')
    
    # clear before each test
    (valid, _) = await self.client.end_all_sessions()
    self.assertTrue(valid)