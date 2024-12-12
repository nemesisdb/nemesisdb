import unittest
from base import NDBSortedStrArrayTest
from ndb.client import ResponseError


class Array(NDBSortedStrArrayTest):
  async def test_create(self):
    await self.arrays.create('arr2', 30)
    len = await self.arrays.len('arr2')
    self.assertEqual(len, 30)


  async def test_set_get(self):
    await self.arrays.create('arr8', 3)
        
    await self.arrays.set('arr8', 'b')
    await self.arrays.set('arr8', 'z')
    await self.arrays.set('arr8', 'a')

    output = await self.arrays.get_rng('arr8', 0)
    self.assertListEqual(output, ['a', 'b', 'z'])


if __name__ == "__main__":
  unittest.main()