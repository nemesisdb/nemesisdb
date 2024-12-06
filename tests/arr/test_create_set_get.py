import unittest
from base import NDBArrayTest, ResponseError


class Array(NDBArrayTest):

  async def test_create_default_size(self):
    await self.client.arr_create('arr1')
    len = await self.client.arr_len('arr1')
    self.assertGreater(len, 0)


  async def test_create_custom_size(self):
    await self.client.arr_create('arr2', 30)
    len = await self.client.arr_len('arr2')
    self.assertEqual(len, 30)


  async def test_set_get(self):
    data1 = {'x':1, 'y':2}
    await self.client.arr_create('arr3', 10)
    
    await self.client.arr_set('arr3', 0, data1)
    output = await self.client.arr_get('arr3', 0)
    self.assertDictEqual(output, data1)

    data2 = {'a':1, 'b':2}
    await self.client.arr_set('arr3', 1, data2)
    output = await self.client.arr_get('arr3', 1)
    self.assertDictEqual(output, data2)


  async def test_overwrite(self):
    data1 = {'x':1, 'y':2}
    await self.client.arr_create('arr4', 10)
    
    await self.client.arr_set('arr4', 0, data1)
    output = await self.client.arr_get('arr4', 0)
    self.assertDictEqual(output, data1)

    data2 = {'a':1, 'b':2}
    await self.client.arr_set('arr4', 0, data2)
    output = await self.client.arr_get('arr4', 0)
    self.assertDictEqual(output, data2)


  async def test_set_bounds(self):
    await self.client.arr_create('arr5', 5)
    
    with self.assertRaises(ResponseError):
      await self.client.arr_set('arr5', 10, {'x':1, 'y':2})

  
  async def test_get_bounds(self):
    await self.client.arr_create('arr6', 5)
    await self.client.arr_set('arr6', 0, {'x':1, 'y':2})

    with self.assertRaises(ResponseError):
      await self.client.arr_get('arr6', 10)


if __name__ == "__main__":
  unittest.main()