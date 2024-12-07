import unittest
from base import NDBArrayTest, ResponseError


class Swap(NDBArrayTest):

  async def test_entire(self):
    data = [{'k0':0}, {'k1':1}, {'k2':2}, {'k3':3}]
    
    await self.client.arr_create('arr1', len(data))
    await self.client.arr_set_rng('arr1', 0, data)
    
    values = await self.client.arr_get_rng('arr1', 0, len(data)-1)

    # swap {'k0':0} with {'k3':3}
    await self.client.arr_swap('arr1', 0, len(data)-1)
    
    swappedValues = await self.client.arr_get_rng('arr1', 0, len(data)-1)

    self.assertDictEqual(swappedValues[0], values[len(data)-1])


if __name__ == "__main__":
  unittest.main()