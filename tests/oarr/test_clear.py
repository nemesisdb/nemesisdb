import unittest
from base import ObjArrayTest


class Clear(ObjArrayTest):

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


  async def test_clear_all_no_stop(self):
    data = [{'k0':0}, {'k1':1}, {'k2':2}, {'k3':3}, {'k4':4}]
    
    await self.arrays.create('arr2', 5)
    await self.arrays.set_rng('arr2', data)

    # clear all (default when no stop is set)
    await self.arrays.clear('arr2', start=0)

    values = await self.arrays.get_rng('arr2', start=0)
    self.assertEqual(len(values), 0)


  async def test_clear_some_no_stop(self):
    data = [{'k0':0}, {'k1':1}, {'k2':2}, {'k3':3}, {'k4':4}]
    
    await self.arrays.create('arr2', 5)
    await self.arrays.set_rng('arr2', data)

    # clear from 2 to end
    await self.arrays.clear('arr2', start=2)

    values = await self.arrays.get_rng('arr2', start=0)
    self.assertEqual(len(values), 2)
    self.assertDictEqual(values[0], {'k0':0})
    self.assertDictEqual(values[1], {'k1':1})

if __name__ == "__main__":
  unittest.main()