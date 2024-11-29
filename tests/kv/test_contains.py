import unittest
from base import NDBTest


class Contains(NDBTest):

  async def test_exists_single(self):
    input = {'i':100}

    await self.client.set(input)

    output = await self.client.contains(('i',))    
    self.assertListEqual(output, ['i'])


  async def test_exists_multiple(self):
    input = {'i':100, 'j':393}

    await self.client.set(input)

    output = await self.client.contains(('i','j'))
    self.assertListEqual(output, ['i','j'])


  async def test_notexist_single(self):
    input = {'i':100}

    await self.client.set(input)

    output = await self.client.contains(('x',))
    self.assertListEqual(output, [])


  async def test_notexist_multiple(self):
    input = {'i':100}

    await self.client.set(input)
    
    output = await self.client.contains(('x','y'))
    self.assertListEqual(output, [])


  async def test_mix(self):
    input = {'i':100, 'j':393}

    await self.client.set(input)

    output = await self.client.contains(('i','y','j'))
    self.assertListEqual(output, ['i','j'])


if __name__ == "__main__":
  unittest.main()