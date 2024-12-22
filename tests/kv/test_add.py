import unittest
from base import KvTest



class Add(KvTest):
  # Minimal tests for add because this is essentially set but 
  # does not overwrite

  async def test_new_key(self):
    input = {'i':100}

    await self.kv.add(input)

    output = await self.kv.get(key='i')
    self.assertEqual(output, input['i'])


  async def test_no_overwrite(self):
    input = {'i':100}

    await self.kv.add(input)

    output = await self.kv.get(key='i')
    self.assertEqual(output, input['i'])

    # call add again, confirm value does not change
    await self.kv.add({'i':200})

    output = await self.kv.get(key='i')
    self.assertEqual(output, input['i'])


if __name__ == "__main__":
  unittest.main()