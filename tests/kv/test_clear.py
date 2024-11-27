import unittest
from base import NDBTest


class Clear(NDBTest):
  
  async def test_clear(self):
    input = {'i':0, 's':'str', 'b':True, 'd':1.5}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    (valid, output) = await self.client.get(tuple(input.keys()))
    self.assertTrue(valid)
    self.assertDictEqual(output, input)

    (valid, clearedCount) = await self.client.clear()
    self.assertTrue(valid)
    self.assertEqual(clearedCount, len(input))

    (valid, output) = await self.client.get(tuple(input.keys()))
    self.assertTrue(valid)
    self.assertDictEqual(output, {})


  async def test_clear_nokeys(self):
    (valid, clearedCount) = await self.client.clear()
    self.assertTrue(valid)
    self.assertEqual(clearedCount, 0)


if __name__ == "__main__":
  unittest.main()