import unittest
from base import NDBTest


class SetGet(NDBTest):
  
  async def test_scalar(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    (valid, output) = await self.client.get(tuple(input.keys()))
    
    self.assertTrue(valid)
    self.assertDictEqual(output, input)


  async def test_object(self):
    input = {'o':{'a':'asda', 'b':10}}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    (valid, output) = await self.client.get(('o',))
    
    self.assertTrue(valid)
    self.assertDictEqual(output, input)


  async def test_overwrite(self):
    valid = await self.client.set({'overwrite_me':100})
    self.assertTrue(valid)

    (valid, output) = await self.client.get(('overwrite_me',))
    self.assertTrue(valid)
    self.assertDictEqual(output, {'overwrite_me':100})

    # overwrite
    valid = await self.client.set({'overwrite_me':200})
    self.assertTrue(valid)

    (valid, output) = await self.client.get(('overwrite_me',))
    self.assertTrue(valid)
    self.assertDictEqual(output, {'overwrite_me':200})


  async def test_key_not_present(self):
    (valid, output) = await self.client.get(('key_does_not_exist',))
    
    self.assertTrue(valid)
    self.assertEqual(len(output), 0)


  async def test_set_no_keys(self):
    valid = await self.client.set({}) # having empty keys is valid
    self.assertTrue(valid)


  async def test_get_no_keys(self):
    (valid, output) = await self.client.get(tuple())
    self.assertTrue(valid)
    self.assertEqual(len(output), 0)


if __name__ == "__main__":
  unittest.main()