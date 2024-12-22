import unittest
from base import KvTest


class Count(KvTest):
  
  async def test_count(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    await self.kv.set(input)

    count = await self.kv.count()
    self.assertEqual(count, len(input))
    


if __name__ == "__main__":
  unittest.main()