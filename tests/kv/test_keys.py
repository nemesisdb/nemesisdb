import unittest
from base import KvTest


class Keys(KvTest):
  
  async def test_keys(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    await self.client.kv_set(input)

    keys = await self.client.kv_keys()
    self.assertListEqual(keys, list(input.keys()))
    
    

if __name__ == "__main__":
  unittest.main()