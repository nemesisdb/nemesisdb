import unittest
from base import NDBArrayTest


class Swap(NDBArrayTest):

  async def test_exist(self):
    await self.arrays.create('arr1', 5)
    exist = await self.arrays.exist('arr1')
    self.assertTrue(exist)

  async def test_not_exist(self):
    exist = await self.arrays.exist('arr_asdbcdef')
    self.assertFalse(exist)


if __name__ == "__main__":
  unittest.main()