import unittest
from base import NDBStrArrayTest
from ndb.client import ResponseError


class Array(NDBStrArrayTest):
  async def test_create(self):
    await self.arrays.create('arr2', 30)
    len = await self.arrays.len('arr2')
    self.assertEqual(len, 30)


  async def test_set_get(self):
    data1 = "hello"
    await self.arrays.create('arr3', 10)
    
    await self.arrays.set('arr3', 0, data1)
    output = await self.arrays.get('arr3', 0)
    self.assertEqual(output, data1)

    data2 = "world"
    await self.arrays.set('arr3', 1, data2)
    output = await self.arrays.get('arr3', 1)
    self.assertEqual(output, data2)


  async def test_overwrite(self):
    await self.arrays.create('arr4', 1)
    
    await self.arrays.set('arr4', 0, "hello world")
    output = await self.arrays.get('arr4', 0)
    self.assertEqual(output, "hello world")

    await self.arrays.set('arr4', 0, "Hello World")
    output = await self.arrays.get('arr4', 0)
    self.assertEqual(output, "Hello World")
    
    
  async def test_invalid_item_type(self):
    await self.arrays.create('arr7', 5)

    with self.assertRaises(ResponseError):
      await self.arrays.set('arr7', 0, {'a':'b'})

    with self.assertRaises(ResponseError):
      await self.arrays.set('arr7', 0, 0.5)

    with self.assertRaises(ResponseError):
      await self.arrays.set('arr7', 0, [])



if __name__ == "__main__":
  unittest.main()