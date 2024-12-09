import unittest
from base import NDBArrayTest
from ndb.arrays import OArrays


class Swap(NDBArrayTest):

  async def test_entire(self):
    data = [{'k0':0}, {'k1':1}, {'k2':2}, {'k3':3}]
    
    arrays = OArrays(self.client)

    await arrays.oarr_create('arr1', len(data))
    await arrays.oarr_set_rng('arr1', 0, data)
    
    values = await arrays.oarr_get_rng('arr1', 0, len(data))

    # swap {'k0':0} with {'k3':3}
    await arrays.oarr_swap('arr1', 0, len(data)-1)
    
    swappedValues = await arrays.oarr_get_rng('arr1', 0, len(data))

    self.assertDictEqual(swappedValues[0], values[len(data)-1])


if __name__ == "__main__":
  unittest.main()