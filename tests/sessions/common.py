import sys
from unittest import IsolatedAsyncioTestCase

sys.path.append('../../apis/python')
from ndb.sessionclient import SessionClient


class NDBSessionTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = SessionClient()
    
    connected = await self.client.open('ws://127.0.0.1:1987')
    self.assertTrue(connected, 'Connection failed')
    
    # clear before each test
    (valid, _) = await self.client.end_all_sessions()
    self.assertTrue(valid)