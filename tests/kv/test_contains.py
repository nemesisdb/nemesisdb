import unittest
import sys
sys.path.append('../')
from base import NDBTest


class Contains(NDBTest):

  async def test_exists_single(self):
    input = {'i':100}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    (valid, output) = await self.client.contains(('i',))
    
    self.assertTrue(valid)
    self.assertListEqual(output, ['i'])


  async def test_exists_multiple(self):
    input = {'i':100, 'j':393}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    (valid, output) = await self.client.contains(('i','j'))
    
    self.assertTrue(valid)
    self.assertListEqual(output, ['i','j'])


  async def test_notexist_single(self):
    input = {'i':100}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    (valid, output) = await self.client.contains(('x',))
    
    self.assertTrue(valid)
    self.assertListEqual(output, [])


  async def test_notexist_multiple(self):
    input = {'i':100}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    (valid, output) = await self.client.contains(('x','y'))
    
    self.assertTrue(valid)
    self.assertListEqual(output, [])


  async def test_mix(self):
    input = {'i':100, 'j':393}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    (valid, output) = await self.client.contains(('i','y','j'))
    
    self.assertTrue(valid)
    self.assertListEqual(output, ['i','j'])


if __name__ == "__main__":
  unittest.main()