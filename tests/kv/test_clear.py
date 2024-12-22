import unittest
from base import KvTest


class Clear(KvTest):
  
  async def test_clear(self):
    input = {'i':0, 's':'str', 'b':True, 'd':1.5}

    await self.kv.set(input)

    output = await self.kv.get(keys=tuple(input.keys()))
    self.assertDictEqual(output, input)

    clearedCount = await self.kv.clear()
    self.assertEqual(clearedCount, len(input))

    output = await self.kv.get(keys=tuple(input.keys()))
    self.assertDictEqual(output, {})


  async def test_clear_nokeys(self):
    clearedCount = await self.kv.clear()
    self.assertEqual(clearedCount, 0)


if __name__ == "__main__":
  unittest.main()