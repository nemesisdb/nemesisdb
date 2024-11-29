import unittest
from base import NDBTest


class Keys(NDBTest):
  
  async def test_keys(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    await self.client.set(input)

    keys = await self.client.keys()
    self.assertListEqual(keys, list(input.keys()))
    
    

if __name__ == "__main__":
  unittest.main()