import unittest
from base import NDBTest


class Remove(NDBTest):
  
  async def test_rmv_one(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    valid = await self.client.rmv(('a',))
    self.assertTrue(valid)

    (valid, output) = await self.client.contains(('a',))
    self.assertListEqual(output, [])


  async def test_rmv_multiple(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    valid = await self.client.rmv(('a','c'))
    self.assertTrue(valid)

    (valid, output) = await self.client.contains(('a','b','c','d'))
    self.assertListEqual(output, ['b','d'])


  async def test_rmv_not_exist(self):
    #note: removing a key that does not exist is not an error, it is ignored
    valid = await self.client.rmv(('not_exist',))
    self.assertTrue(valid)


if __name__ == "__main__":
  unittest.main()