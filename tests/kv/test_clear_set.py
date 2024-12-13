import unittest
from base import KvTest


class ClearSet(KvTest):
  
  async def test_clear_set(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    await self.client.kv_set(input)

    output = await self.client.kv_get(keys=tuple(input.keys()))
    self.assertDictEqual(output, input)

    clearedCount = await self.client.kv_clear_set({'e':'asda', 'f':False})
    self.assertEqual(clearedCount, len(input))

    output = await self.client.kv_get(keys=('i','s','b','d', 'e', 'f'))
    self.assertDictEqual(output, {'e':'asda', 'f':False})


  async def test_no_new_keys(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    await self.client.kv_set(input)

    output = await self.client.kv_get(keys=tuple(input.keys()))
    self.assertDictEqual(output, input)

    clearedCount = await self.client.kv_clear_set({})
    self.assertEqual(clearedCount, len(input))

    # the original keys are cleared but we don't set new keys in clear_set()
    output = await self.client.kv_get(keys=('i','s','b','d'))
    self.assertDictEqual(output, {})


if __name__ == "__main__":
  unittest.main()
  