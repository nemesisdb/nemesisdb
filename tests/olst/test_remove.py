import unittest
from base import ObjListTest


class ListRmv(ObjListTest):

  async def test_rmv_head(self):
    name = '1'

    await self.lists.create(name)    
    await self.lists.add(name, [{'a':0}, {'b':0}, {'c':0}])
    size = await self.lists.remove_head(name)
    self.assertEqual(size, 2)

    values = await self.lists.get_rng(name, 0)
    self.assertListEqual([{'b':0}, {'c':0}], values)


  async def test_rmv_tail(self):
    name = '2'

    await self.lists.create(name)    
    await self.lists.add(name, [{'a':0}, {'b':0}, {'c':0}])
    size = await self.lists.remove_tail(name)
    self.assertEqual(size, 2)

    values = await self.lists.get_rng(name, 0)
    self.assertListEqual([{'a':0}, {'b':0}], values)


  async def test_rmv_mid(self):
    name = '3'

    await self.lists.create(name)    
    await self.lists.add(name, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])

    size = await self.lists.remove(name, start=1, stop=3)
    self.assertEqual(size, 2)

    values = await self.lists.get_rng(name, 0)
    self.assertListEqual([{'a':0}, {'d':0}], values)


  async def test_rmv_all_but_head(self):
    name = '4'

    await self.lists.create(name)    
    await self.lists.add(name, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])

    size = await self.lists.remove(name, start=1)
    self.assertEqual(size, 1)

    values = await self.lists.get_rng(name, 0)
    self.assertListEqual([{'a':0}], values)


  async def test_rmv_all_but_tail(self):
    name = '4'

    await self.lists.create(name)    
    await self.lists.add(name, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])

    size = await self.lists.remove(name, start=0, stop=3)
    self.assertEqual(size, 1)

    values = await self.lists.get_rng(name, 0)
    self.assertListEqual([{'d':0}], values)


if __name__ == "__main__":
  unittest.main()