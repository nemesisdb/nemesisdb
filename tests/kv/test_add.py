import unittest
from base import NDBTest



class Add(NDBTest):
  # Minimal tests for add because this is essentially set but 
  # does not overwrite

  async def test_new_key(self):
    input = {'i':100}

    await self.client.add(input)

    output = await self.client.get(('i',))    
    self.assertDictEqual(output, input)


  async def test_no_overwrite(self):
    input = {'i':100}

    await self.client.add(input)

    output = await self.client.get(('i',))
    self.assertDictEqual(output, input)

    # call add again, confirm value does not change
    await self.client.add({'i':200})

    output = await self.client.get(('i',))
    self.assertDictEqual(output, input)


if __name__ == "__main__":
  unittest.main()