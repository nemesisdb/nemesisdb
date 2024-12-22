import unittest
from base import KvTest


class SetGet(KvTest):
  
  async def test_scalar(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    await self.kv.set(input)
    
    output = await self.kv.get(keys=tuple(input.keys()))        
    self.assertDictEqual(output, input)


  async def test_object(self):
    input = {'o':{'a':'asda', 'b':10}}

    await self.kv.set(input)
    
    output = await self.kv.get(key='o')
    self.assertEqual(output, input['o'])


  async def test_overwrite(self):
    await self.kv.set({'overwrite_me':100})
    
    output = await self.kv.get(key='overwrite_me')
    self.assertEqual(output, 100)
    
    await self.kv.set({'overwrite_me':200})
    
    output = await self.kv.get(key='overwrite_me')
    self.assertEqual(output, 200)


  async def test_key_not_present(self):
    output = await self.kv.get(key='key_does_not_exist')
    self.assertEqual(output, None)


  async def test_set_no_keys(self):
    await self.kv.set({}) # having empty keys is valid
    

  async def test_get_no_keys(self):
    output = await self.kv.get(keys=tuple())
    self.assertDictEqual(output, {})
    

  async def test_get_key_keys(self):
    with self.assertRaises(ValueError):
      await self.kv.get(keys=tuple('a'), key='a')


if __name__ == "__main__":
  unittest.main()