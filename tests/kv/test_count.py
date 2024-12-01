import unittest
from base import NDBTest


class Count(NDBTest):
  
  async def test_count(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    await self.client.kv_set(input)

    count = await self.client.kv_count()
    self.assertEqual(count, len(input))
    


if __name__ == "__main__":
  unittest.main()