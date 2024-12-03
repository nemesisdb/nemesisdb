import unittest
from base import NDBTest


class Clear(NDBTest):
  
  async def test_clear(self):
    input = {'i':0, 's':'str', 'b':True, 'd':1.5}

    await self.client.kv_set(input)

    output = await self.client.kv_get(keys=tuple(input.keys()))
    self.assertDictEqual(output, input)

    clearedCount = await self.client.kv_clear()
    self.assertEqual(clearedCount, len(input))

    output = await self.client.kv_get(keys=tuple(input.keys()))
    self.assertDictEqual(output, {})


  async def test_clear_nokeys(self):
    clearedCount = await self.client.kv_clear()
    self.assertEqual(clearedCount, 0)


if __name__ == "__main__":
  unittest.main()