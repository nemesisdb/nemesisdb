from unittest import IsolatedAsyncioTestCase
from ndb.client import NdbClient
from ndb.arrays import OArrays, IArrays


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
    
    self.arrays = OArrays(self.client)

    # clear before each test
    await self.arrays.oarr_delete_all()


class NDBIArrayTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = NdbClient()
    
    connected = await self.client.open('ws://127.0.0.1:1987')
    self.assertTrue(connected, 'Connection failed')
    
    self.arrays = IArrays(self.client)

    # clear before each test
    await self.arrays.iarr_delete_all()