import unittest
from base import NDBArrayTest, ResponseError


class Clear(NDBArrayTest):

  async def test_clear(self):
    data = [{'k0':0}, {'k1':1}, {'k2':2}, {'k3':3}, {'k4':4}]
    
    await self.client.arr_create('arr1', 5)
    await self.client.arr_set_rng('arr1', 0, data)

    # clear k1 to k3 inclusive
    await self.client.arr_clear('arr1', 1, 4)

    emptyValues = await self.client.arr_get_rng('arr1', 0, 5)
    self.assertDictEqual(emptyValues[0], {'k0':0})
    self.assertDictEqual(emptyValues[1], {})
    self.assertDictEqual(emptyValues[2], {})
    self.assertDictEqual(emptyValues[3], {})
    self.assertDictEqual(emptyValues[4], {'k4':4})


if __name__ == "__main__":
  unittest.main()