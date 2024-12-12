import unittest
from base import NDBIArrayTest
from ndb.client import ResponseError


class Array(NDBIArrayTest):
  async def test_create(self):
    await self.arrays.create('arr2', 30)
    len = await self.arrays.capacity('arr2')
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
    
    await self.arrays.set('arr3', data1, 0)
    output = await self.arrays.get('arr3', 0)
    self.assertEqual(output, data1)

    data2 = 2
    await self.arrays.set('arr3', data2, 1)
    output = await self.arrays.get('arr3', 1)
    self.assertEqual(output, data2)


  async def test_set_get_nopos(self):
    data1 = 1
    await self.arrays.create('arr3', 10)
    
    await self.arrays.set('arr3', data1)
    output = await self.arrays.get('arr3', 0)
    self.assertEqual(output, data1)

    data2 = 2
    await self.arrays.set('arr3', data2)
    output = await self.arrays.get('arr3', 1)
    self.assertEqual(output, data2)


  async def test_overwrite(self):
    data1 = 1
    await self.arrays.create('arr4', 2)
    
    await self.arrays.set('arr4', data1)
    output = await self.arrays.get('arr4', 0)
    self.assertEqual(output, data1)

    data2 = 2
    await self.arrays.set('arr4', data2, pos=0)
    output = await self.arrays.get('arr4', 0)
    self.assertEqual(output, data2)


  async def test_set_bounds(self):
    await self.arrays.create('arr5', 5)
    
    with self.assertRaises(ResponseError):
      await self.arrays.set('arr5', 123, pos=10)

  
  async def test_get_bounds(self):
    await self.arrays.create('arr6', 5)
    with self.assertRaises(ResponseError):
      await self.arrays.get('arr6', 10)

    
  async def test_invalid_item_type(self):
    await self.arrays.create('arr7', 5)

    with self.assertRaises(ResponseError):
      await self.arrays.set('arr7', {'a':'b'})

    with self.assertRaises(ResponseError):
      await self.arrays.set('arr7', 0.5)

    with self.assertRaises(ResponseError):
      await self.arrays.set('arr7', "")



if __name__ == "__main__":
  unittest.main()