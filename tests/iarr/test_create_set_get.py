import unittest
from base import NDBIArrayTest
from ndb.client import ResponseError


class Array(NDBIArrayTest):

  async def test_create(self):

    await self.arrays.iarr_create('arr2', 30)
    len = await self.arrays.iarr_len('arr2')
    self.assertEqual(len, 30)

  
  async def test_create_invalid_size(self):
    # caught by PyAPI
    with self.assertRaises(ValueError):
      await self.arrays.iarr_create('arr_', 0)

    with self.assertRaises(ValueError): 
      await self.arrays.iarr_create('arr_', -1)

    with self.assertRaises(ResponseError): 
      await self.arrays.iarr_create('arr_', 10000)


  async def test_set_get(self):
    data1 = 1
    await self.arrays.iarr_create('arr3', 10)
    
    await self.arrays.iarr_set('arr3', 0, data1)
    output = await self.arrays.iarr_get('arr3', 0)
    self.assertEqual(output, data1)

    data2 = 2
    await self.arrays.iarr_set('arr3', 1, data2)
    output = await self.arrays.iarr_get('arr3', 1)
    self.assertEqual(output, data2)


  # async def test_overwrite(self):
  #   data1 = {'x':1, 'y':2}
  #   await self.arrays.iarr_create('arr4', 10)
    
  #   await self.arrays.iarr_set('arr4', 0, data1)
  #   output = await self.arrays.iarr_get('arr4', 0)
  #   self.assertDictEqual(output, data1)

  #   data2 = {'a':1, 'b':2}
  #   await self.arrays.iarr_set('arr4', 0, data2)
  #   output = await self.arrays.iarr_get('arr4', 0)
  #   self.assertDictEqual(output, data2)


  # async def test_set_bounds(self):
  #   await self.arrays.iarr_create('arr5', 5)
    
  #   with self.assertRaises(ResponseError):
  #     await self.arrays.iarr_set('arr5', 10, {'x':1, 'y':2})

  
  # async def test_get_bounds(self):
  #   await self.arrays.iarr_create('arr6', 5)
  #   with self.assertRaises(ResponseError):
  #     await self.arrays.iarr_get('arr6', 10)


if __name__ == "__main__":
  unittest.main()