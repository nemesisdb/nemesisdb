import unittest
from base import NDBArrayTest, ResponseError


class Swap(NDBArrayTest):

  async def test_exist(self):
    await self.client.arr_create('arr1', 5)
    exist = await self.client.arr_exist('arr1')
    self.assertTrue(exist)

  async def test_not_exist(self):
    exist = await self.client.arr_exist('arr_asdbcdef')
    self.assertFalse(exist)


if __name__ == "__main__":
  unittest.main()