import unittest
from base import ObjListTest


class ListAdd(ObjListTest):

  #region add

  async def test_add_nopos_empty(self):
    await self.lists.create('l1')
    await self.lists.add('l1', [{'a':0}])

    values = await self.lists.get_rng('l1', 0)
    self.assertEqual(len(values), 1)
    self.assertListEqual([{'a':0}], values)


  async def test_add_nopos(self):
    await self.lists.create('l2')
    await self.lists.add('l2', [{'a':0}])
    await self.lists.add('l2', [{'b':0}])

    values = await self.lists.get_rng('l2', 0)
    self.assertEqual(len(values), 2)
    self.assertListEqual([{'a':0}, {'b':0}], values)

  
  async def test_add_pos_head(self):
    await self.lists.create('l3')
    await self.lists.add('l3', [{'b':0}])

    await self.lists.add('l3', [{'a':0}], pos=0)
    values = await self.lists.get_rng('l3', 0)
    self.assertEqual(len(values), 2)
    self.assertListEqual([{'a':0}, {'b':0}], values)


  async def test_add_pos_tail(self):
    name = 'l4'
    await self.lists.create(name)
    await self.lists.add(name, [{'a':0}, {'b':0}])

    await self.lists.add(name, [{'c':0}], pos=2)
    values = await self.lists.get_rng(name, 0)
    self.assertEqual(len(values), 3)
    self.assertListEqual([{'a':0}, {'b':0}, {'c':0}], values)


  async def test_add_pos_insert(self):
    name = 'l5'
    await self.lists.create(name)
    await self.lists.add(name, [{'a':0}, {'c':0}])

    await self.lists.add(name, [{'b':0}], pos=1)
    values = await self.lists.get_rng(name, 0)
    self.assertEqual(len(values), 3)
    self.assertListEqual([{'a':0}, {'b':0}, {'c':0}], values)



  async def test_add_pos_bounds(self):
    name = 'l6'
    await self.lists.create(name)
    await self.lists.add(name, [{'a':0}, {'b':0}])

    await self.lists.add(name, [{'c':0}], pos=100)
    values = await self.lists.get_rng(name, 0)
    self.assertEqual(len(values), 3)
    self.assertListEqual([{'a':0}, {'b':0}, {'c':0}], values)


  async def test_add_pos_invalid(self):
    name = 'l7'
    await self.lists.create(name)
    
    with self.assertRaises(ValueError):
      await self.lists.add(name, [{'a':0}], pos=-1)

    with self.assertRaises(TypeError):
      await self.lists.add(name, [{'a':0}], pos='')

  #endregion


  #region add_head
  async def test_add_head(self):
    name = '8'
    await self.lists.create(name)
    await self.lists.add(name, [{'b':0}])

    await self.lists.add_head(name, [{'a':0}])
    values = await self.lists.get_rng(name, 0)
    self.assertEqual(len(values), 2)
    self.assertListEqual([{'a':0}, {'b':0}], values)
  #endregion


  #region add_tail

  async def test_add_tail(self):
    name = '9'
    await self.lists.create(name)
    await self.lists.add(name, [{'a':0}, {'b':0}])

    await self.lists.add_tail(name, [{'c':0}])
    values = await self.lists.get_rng(name, 0)
    self.assertEqual(len(values), 3)
    self.assertListEqual([{'a':0}, {'b':0}, {'c':0}], values)

  #endregion


if __name__ == "__main__":
  unittest.main()