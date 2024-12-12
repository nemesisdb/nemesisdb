import unittest
from base import NDBSortedIntArrayTest
from ndb.client import ResponseError


class Array(NDBSortedIntArrayTest):
  async def test_create(self):
    await self.arrays.create('arr2', 30)
    len = await self.arrays.len('arr2')
    self.assertEqual(len, 30)

  
  async def test_create_invalid_size(self):
    with self.assertRaises(ValueError): # caught by PyAPI
      await self.arrays.create('arr_', 0)

    with self.assertRaises(ValueError): 
      await self.arrays.create('arr_', -1)

    with self.assertRaises(ResponseError): 
      await self.arrays.create('arr_', 10000)


  async def test_set_get(self):
    data1 = 1
    await self.arrays.create('arr3', 10)
    
    await self.arrays.set('arr3', data1)
    output = await self.arrays.get('arr3', 0)
    self.assertEqual(output, data1)

    data2 = 2
    await self.arrays.set('arr3', data2)
    output = await self.arrays.get('arr3', 1)
    self.assertEqual(output, data2)


  async def test_swap(self):
    with self.assertRaises(NotImplementedError):
      await self.arrays.swap('arr6', 0, 1)

  
  async def test_get_bounds(self):
    await self.arrays.create('arr6', 5)
    with self.assertRaises(ResponseError):
      await self.arrays.get('arr6', 10)

    
  async def test_invalid_item_type(self):
    await self.arrays.create('arr7', 5)

    with self.assertRaises(ResponseError):
      await self.arrays.set('arr7', {'a':'b'})

    with self.assertRaises(ResponseError):
      await self.arrays.set('arr7',  0.5)

    with self.assertRaises(ResponseError):
      await self.arrays.set('arr7', "")



if __name__ == "__main__":
  unittest.main()