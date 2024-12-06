from unittest import IsolatedAsyncioTestCase
from ndb.client import NdbClient, ResponseError


class NDBTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = NdbClient()
    
    connected = await self.client.open('ws://127.0.0.1:1987')
    self.assertTrue(connected, 'Connection failed')
    
    # clear before each test
    await self.client.kv_clear()


class NDBSessionTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = NdbClient()
    
    connected = await self.client.open('ws://127.0.0.1:1987')
    self.assertTrue(connected, 'Connection failed')
    
    # clear before each test
    await self.client.sh_end_all()


class NDBArrayTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = NdbClient()
    
    connected = await self.client.open('ws://127.0.0.1:1987')
    self.assertTrue(connected, 'Connection failed')
    
    # clear before each test
    await self.client.arr_delete_all()
