import unittest
from base import ObjListTest
from ndb.client import ResponseError


class ListSetRng(ObjListTest):

  async def test_all(self):
    name = 'l1'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]
    newData = [{'e':0}, {'f':0}, {'g':0}, {'h':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)

    await self.lists.set_rng(name, newData, start=0)

    values = await self.lists.get_rng(name, start=0)
    self.assertEqual(len(values), len(data))
    self.assertListEqual(newData, values)


  async def test_mid(self):
    name = 'l2'
    data = [{'a':0}, {'b':0}, {'c':0}, {'d':0}]
    newData = [{'a':0}, {'f':0}, {'g':0}, {'d':0}]

    await self.lists.create(name)
    await self.lists.add(name, data)

    await self.lists.set_rng(name, [{'f':0}, {'g':0}], start=1)

    values = await self.lists.get_rng(name, start=0)
    self.assertEqual(len(values), len(data))
    self.assertListEqual(newData, values)


  async def test_bounds(self):
    name = 'l3'
    await self.lists.create(name)

    with self.assertRaises(ResponseError):
      await self.lists.set_rng(name, [{'f':0}, {'g':0}], start=1)

    await self.lists.add(name, [{'a':0}, {'b': 0}])
    with self.assertRaises(ResponseError):
      await self.lists.set_rng(name, [{'f':0}, {'g':0}], start=2)


if __name__ == "__main__":
  unittest.main()