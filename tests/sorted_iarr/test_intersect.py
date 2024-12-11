import unittest
from base import NDBSortedIntArrayTest
from ndb.client import ResponseError


class Array(NDBSortedIntArrayTest):
  async def test_intersect_ordered(self):
    await self.arrays.arr_create('a', 5)
    await self.arrays.arr_create('b', 5)

    a_data = [1,2,3,4,5]
    b_data = [2,3,4]

    await self.arrays.arr_set_rng('a', a_data)
    await self.arrays.arr_set_rng('b', b_data)

    intersect = await self.arrays.arr_intersect('a', 'b')
    self.assertListEqual(intersect, [2,3,4])


  async def test_intersect_unordered(self):
    await self.arrays.arr_create('a', 5)
    await self.arrays.arr_create('b', 5)

    a_data = [5,2,1,4,3]
    b_data = [2,4,3]

    await self.arrays.arr_set_rng('a', a_data)
    await self.arrays.arr_set_rng('b', b_data)

    intersect = await self.arrays.arr_intersect('a', 'b')
    self.assertListEqual(intersect, [2,3,4])


  async def test_same_arrays(self):
    await self.arrays.arr_create('a', 5)
    await self.arrays.arr_create('b', 5)

    with self.assertRaises(ValueError): # caught by Py API
      await self.arrays.arr_intersect('a', 'a')
