import unittest
from base import NDBTest


class Remove(NDBTest):
  
  async def test_rmv_one(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    await self.client.set(input)

    await self.client.rmv(('a',))

    output = await self.client.contains(('a',))
    self.assertListEqual(output, [])


  async def test_rmv_multiple(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    await self.client.set(input)

    await self.client.rmv(('a','c'))

    output = await self.client.contains(('a','b','c','d'))
    self.assertListEqual(output, ['b','d'])


  async def test_rmv_not_exist(self):
    #note: removing a key that does not exist is not an error, it is ignored
    await self.client.rmv(('not_exist',))


if __name__ == "__main__":
  unittest.main()