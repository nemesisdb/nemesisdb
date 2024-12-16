import unittest
from base import ObjListTest


class ListExistDelete(ObjListTest):

  async def test_exist(self):
    name = 'l1'

    self.assertEqual(False, await self.lists.exist(name))    
    await self.lists.create(name)
    self.assertEqual(True, await self.lists.exist(name))


  async def test_delete(self):
    await self.lists.create('l1')
    await self.lists.create('l2')
    self.assertEqual(True, await self.lists.exist('l1'))
    self.assertEqual(True, await self.lists.exist('l2'))
    
    await self.lists.delete('l1')
    self.assertEqual(False, await self.lists.exist('l1'))
    self.assertEqual(True, await self.lists.exist('l2'))


  async def test_delete_all(self):
    for i in range(20):
      await self.lists.create(f'{i}')

    await self.lists.delete_all()
    
    for i in range(20):
      self.assertEqual(False, await self.lists.exist(f'{i}'))    


if __name__ == "__main__":
  unittest.main()