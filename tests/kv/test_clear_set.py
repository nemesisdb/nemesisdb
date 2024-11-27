import unittest
from base import NDBTest


class ClearSet(NDBTest):
  
  async def test_clear_set(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    (valid, output) = await self.client.get(tuple(input.keys()))
    self.assertTrue(valid)
    self.assertDictEqual(output, input)

    (valid, clearedCount) = await self.client.clear_set({'e':'asda', 'f':False})
    self.assertTrue(valid)
    self.assertEqual(clearedCount, len(input))

    (valid, output) = await self.client.get(('i','s','b','d', 'e', 'f'))
    self.assertTrue(valid)
    self.assertDictEqual(output, {'e':'asda', 'f':False})


  async def test_no_new_keys(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    (valid, output) = await self.client.get(tuple(input.keys()))
    self.assertTrue(valid)
    self.assertDictEqual(output, input)

    (valid, clearedCount) = await self.client.clear_set({})
    self.assertTrue(valid)
    self.assertEqual(clearedCount, len(input))

    # the original keys are cleared but we don't set new keys in clear_set()
    (valid, output) = await self.client.get(('i','s','b','d'))
    self.assertTrue(valid)
    self.assertDictEqual(output, {})


if __name__ == "__main__":
  unittest.main()