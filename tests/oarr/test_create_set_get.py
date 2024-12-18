import unittest
from base import ObjArrayTest
from ndb.client import ResponseError


class Array(ObjArrayTest):

  async def test_create(self):

    await self.arrays.create('arr2', 30)
    len = await self.arrays.capacity('arr2')
    self.assertEqual(len, 30)

  
  async def test_create_invalid_size(self):
    # caught by PyAPI
    with self.assertRaises(ValueError):
      await self.arrays.create('arr_', 0)

    with self.assertRaises(ValueError): 
      await self.arrays.create('arr_', -1)

    with self.assertRaises(ResponseError): 
      await self.arrays.create('arr_', 100000)


  async def test_set_get(self):
    data1 = {'x':1, 'y':2}
    await self.arrays.create('arr3', 10)
    
    await self.arrays.set('arr3', data1)
    output = await self.arrays.get('arr3', 0)
    self.assertDictEqual(output, data1)

    data2 = {'a':1, 'b':2}
    await self.arrays.set('arr3', data2)
    output = await self.arrays.get('arr3', 1)
    self.assertDictEqual(output, data2)


  async def test_overwrite(self):
    data1 = {'x':1, 'y':2}
    await self.arrays.create('arr4', 10)
    
    await self.arrays.set('arr4', data1)
    output = await self.arrays.get('arr4', 0)
    self.assertDictEqual(output, data1)

    data2 = {'a':1, 'b':2}
    await self.arrays.set('arr4', data2, pos=0)
    output = await self.arrays.get('arr4', 0)
    self.assertDictEqual(output, data2)


  async def test_set_bounds(self):
    await self.arrays.create('arr5', 5)
    
    with self.assertRaises(ResponseError):
      await self.arrays.set('arr5', {'x':1, 'y':2}, pos=6)

  
  async def test_get_bounds(self):
    await self.arrays.create('arr6', 5)
    with self.assertRaises(ResponseError):
      await self.arrays.get('arr6', 10)


if __name__ == "__main__":
  unittest.main()