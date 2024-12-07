import unittest
from base import NDBArrayTest, ResponseError


class SetRange(NDBArrayTest):

  async def test_entire(self):
    size = 10
    data = list()

    for i in range(0, size):
      data.append({f'k{i}':i})
    
    await self.client.arr_create('arr1', size)

    await self.client.arr_set_rng('arr1', 0, data)
    values = await self.client.arr_get_rng('arr1', 0, size-1)

    self.assertEqual(len(values), size)

    for i in range(0, len(values)):
      self.assertDictEqual(values[i], {f'k{i}':i})



  async def test_partial(self):
    size = 10
    setSize = 5
    data = list()

    for i in range(0, setSize):
      data.append({f'k{i}':i})
    
    await self.client.arr_create('arr2', size)

    await self.client.arr_set_rng('arr2', 0, data)
    values = await self.client.arr_get_rng('arr2', 0, setSize-1)

    self.assertEqual(len(values), setSize)

    for i in range(0, len(values)):
      self.assertDictEqual(values[i], {f'k{i}':i})


  async def test_partial_middle(self):
    # This test start in the middle of the range, unlike the two above which start from 0
    # Create data [{'k0':0}, {'k1':1}, ... {'k9':9}]
    # Create array size 10
    # Set from data[2] to data[6] to array[2] to array[6]
    size = 10
    setSize = 5
    setStart = 2
    data = list()

    for i in range(0, size):
      data.append({f'k{i}':i})
    
    await self.client.arr_create('arr3', size)

    await self.client.arr_set_rng('arr3', setStart, data[setStart:setStart+setSize])

    values = await self.client.arr_get_rng('arr3', setStart, setStart+setSize-1)
    self.assertEqual(len(values), setSize)
    
    for i in range(0, len(values)):
      self.assertDictEqual(values[i], {f'k{i+setStart}':i+setStart})


  async def test_pos_bounds(self):
    await self.client.arr_create('arr4', 5)
    
    with self.assertRaises(ResponseError):
      await self.client.arr_set_rng('arr4', 5, [{'a':0}])

    with self.assertRaises(ResponseError):
      await self.client.arr_set_rng('arr4', 7, [{'a':0}])


if __name__ == "__main__":
  unittest.main()