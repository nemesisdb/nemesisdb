import unittest
from base import NDBArrayTest
from ndb.client import ResponseError


class RangeTest(NDBArrayTest):
  async def asyncSetUp(self):
    await super().asyncSetUp()    

    self.size = 20

    await self.arrays.oarr_create('arr', self.size)
    size = await self.arrays.oarr_len('arr')
    self.assertEqual(size, self.size)

    for i in range(0, self.size):
      await self.arrays.oarr_set('arr', i, {f'k{i}':i})


class GetRange(RangeTest):

  async def test_small(self):
    rangeSize = self.size//4
    stop = rangeSize 

    values = await self.arrays.oarr_get_rng('arr', 0, stop)
    self.assertEqual(len(values), rangeSize)

    for i in range(0, rangeSize):
      self.assertDictEqual(values[i], {f'k{i}':i})


  async def test_larger(self):
    rangeSize = self.size//2
    stop = rangeSize

    values = await self.arrays.oarr_get_rng('arr', 0, stop)
    self.assertEqual(len(values), rangeSize)

    for i in range(0, rangeSize):
      self.assertDictEqual(values[i], {f'k{i}':i})


  async def test_start_stop_eq(self):
    values = await self.arrays.oarr_get_rng('arr', 0, 0)
    self.assertEqual(len(values), 0)

  
  async def test_rng_one(self):
    values = await self.arrays.oarr_get_rng('arr', 0, 1)
    self.assertEqual(len(values), 1)
    self.assertDictEqual(values[0], {'k0':0})


  async def test_stop_past_end(self):
    # stop is capped to end()
    values = await self.arrays.oarr_get_rng('arr', 0, self.size+1)
    self.assertEqual(len(values), self.size)


  async def test_start_only(self):
    # stop is capped to end()
    values = await self.arrays.oarr_get_rng('arr', 0)
    self.assertEqual(len(values), self.size)


  async def test_start_gt_stop(self):
    with self.assertRaises(ValueError): # caught by the PyAPI before being sent
      await self.arrays.oarr_get_rng('arr', 5, 0)


  async def test_negative(self):
    with self.assertRaises(ResponseError): # problem is negative start index
      await self.arrays.oarr_get_rng('arr', -1, 3)


if __name__ == "__main__":
  unittest.main()