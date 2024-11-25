import unittest
import sys
sys.path.append('../')
from base import NDBTest


class Count(NDBTest):
  
  async def test_count(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    (valid, count) = await self.client.count()
    self.assertTrue(valid)
    self.assertEqual(count, len(input))
    




if __name__ == "__main__":
  unittest.main()