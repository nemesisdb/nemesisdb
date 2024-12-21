import unittest
from base import ObjListTest
from ndb.client import ResponseError


class ListGet(ObjListTest):


  #region GetRange

  async def test_start_head(self):
    name = 'l1'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)

    values = await self.lists.get_rng(name, start=0)
    self.assertEqual(len(values), len(data))
    self.assertListEqual(data, values)


  async def test_start_tail(self):
    name = 'l2'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)

    values = await self.lists.get_rng(name, start=3)
    self.assertEqual(len(values), 1)
    self.assertListEqual([{'d':0}], values)


  async def test_start_mid(self):
    name = 'l3'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)

    values = await self.lists.get_rng(name, start=1)
    self.assertEqual(len(values), 3)
    self.assertListEqual([{'b':0}, {'c':0}, {'d':0}], values)


  async def test_start_end_all(self):
    name = 'l4'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)

    values = await self.lists.get_rng(name, start=0, stop=4)
    self.assertEqual(len(values), len(data))
    self.assertListEqual(data, values)


  async def test_start_end_mid(self):
    name = 'l5'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)

    values = await self.lists.get_rng(name, start=1, stop=3)
    self.assertEqual(len(values), 2)
    self.assertListEqual([{'b':0}, {'c':0}], values)


  async def test_end_bounds(self):
    name = 'l6'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)

    values = await self.lists.get_rng(name, start=0, stop=10)
    self.assertEqual(len(values), len(data))
    self.assertListEqual(data, values)


  async def test_start_bounds(self):
    name = 'l7'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)
    values = await self.lists.get_rng(name, start=10)
    self.assertListEqual(values, [])


  async def test_start_empty_bounds(self):
    name = 'l8'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]

    await self.lists.create(name)
    values = await self.lists.get_rng(name, start=10)
    self.assertListEqual(values, [])


  async def test_stop_lt_start(self):
    with self.assertRaises(ValueError):
      await self.lists.get_rng('abc', start=10,stop=9)

  #endregion


  #region Get
  async def test_pos_head_tail(self):
    name = 'l9'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)

    value = await self.lists.get(name, pos=0)
    self.assertDictEqual({'a':0}, value)

    value = await self.lists.get(name, pos=3)
    self.assertDictEqual({'d':0}, value)


  async def test_pos_mid(self):
    name = 'l10'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)

    value = await self.lists.get(name, pos=2)
    self.assertDictEqual({'c':0}, value)


  async def test_pos_bounds(self):
    name = 'l11'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)

    with self.assertRaises(ResponseError):
      await self.lists.get(name, pos=12)

  #endregion


  #region get_head
  async def test_get_head(self):
    name = 'l12'

    await self.lists.create(name)
    await self.lists.add(name, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])

    value = await self.lists.get_head(name)
    self.assertDictEqual({'a':0}, value)
  #endregion


  #region get_tail
  async def test_get_tail(self):
    name = 'l13'

    await self.lists.create(name)
    await self.lists.add(name, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])

    value = await self.lists.get_tail(name)
    self.assertDictEqual({'d':0}, value)
  #endregion


if __name__ == "__main__":
  unittest.main()