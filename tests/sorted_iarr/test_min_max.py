import unittest
from base import SortedIntArrayTest
from ndb.client import ResponseError


class Array(SortedIntArrayTest):
  async def test_min_1(self):
    await self.arrays.create('arr1', 6)
    await self.arrays.set_rng('arr1', [20,15,30,80,5,26])
    mins = await self.arrays.min('arr1')
    self.assertEqual(len(mins), 1)
    self.assertEqual(mins[0], 5)


  async def test_min_3(self):
    await self.arrays.create('arr2', 6)
    await self.arrays.set_rng('arr2', [20,15,30,80,5,26])
    mins = await self.arrays.min('arr2', n=3)
    self.assertEqual(len(mins), 3)
    self.assertListEqual(mins, [5,15,20])

  
  async def test_min_bounds(self):
    await self.arrays.create('arr3', 6)
    with self.assertRaises(ResponseError):
      await self.arrays.min('arr3', 10)

    with self.assertRaises(ValueError):
      await self.arrays.min('arr3', 0)

  # max
  async def test_max_1(self):
    await self.arrays.create('arr4', 6)
    await self.arrays.set_rng('arr4', [20,15,30,80,5,26])
    maxes = await self.arrays.max('arr4')
    self.assertEqual(len(maxes), 1)
    self.assertEqual(maxes[0], 80)


  async def test_max_3(self):
    await self.arrays.create('arr5', 6)
    await self.arrays.set_rng('arr5', [20,15,30,80,5,26])
    maxes = await self.arrays.max('arr5', n=3)
    self.assertEqual(len(maxes), 3)
    self.assertEqual(maxes, [80,30,26])


  async def test_max_bounds(self):
    await self.arrays.create('arr6', 6)
    with self.assertRaises(ResponseError):
      await self.arrays.max('arr6', 10)

    with self.assertRaises(ValueError):
      await self.arrays.max('arr6', 0)

      
if __name__ == "__main__":
  unittest.main()