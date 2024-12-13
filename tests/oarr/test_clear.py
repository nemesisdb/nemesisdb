import unittest
from base import NDBArrayTest


class Clear(NDBArrayTest):

  async def test_clear(self):
    data = [{'k0':0}, {'k1':1}, {'k2':2}, {'k3':3}, {'k4':4}]
    
    await self.arrays.create('arr1', 5)
    await self.arrays.set_rng('arr1', data)

    # clear k1 to k3 inclusive
    await self.arrays.clear('arr1', 1, 4)

    values = await self.arrays.get_rng('arr1', start=0)
    self.assertEqual(len(values), 2)
    self.assertDictEqual(values[0], {'k0':0})
    self.assertDictEqual(values[1], {'k4':4})


if __name__ == "__main__":
  unittest.main()