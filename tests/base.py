from unittest import IsolatedAsyncioTestCase
from ndb.client import NdbClient
from ndb.arrays import ObjArrays, IntArrays, SortedIntArrays, StringArrays, SortedStrArrays
from ndb.lists import ObjLists
from ndb.kv import KV

class NDBTest(IsolatedAsyncioTestCase):
  async def asyncSetUp(self):
    self.client = NdbClient()
    await self.client.open('ws://127.0.0.1:1987')    
    

class KvTest(NDBTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()
    await self.client.open('ws://127.0.0.1:1987')    
    
    self.kv = KV(self.client)
    await self.kv.clear()


class ObjArrayTest(NDBTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()
    self.arrays = ObjArrays(self.client)
    await self.arrays.delete_all()


class IArrayTest(NDBTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()    
    self.arrays = IntArrays(self.client)
    await self.arrays.delete_all()


class SortedIntArrayTest(NDBTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()
    self.arrays = SortedIntArrays(self.client)
    await self.arrays.delete_all()


class StrArrayTest(NDBTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()    
    self.arrays = StringArrays(self.client)
    await self.arrays.delete_all()
    

class SortedStrArrayTest(NDBTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()
    self.arrays = SortedStrArrays(self.client)
    await self.arrays.delete_all()


class ObjListTest(NDBTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()
    self.lists = ObjLists(self.client)
    await self.lists.delete_all() # TODO