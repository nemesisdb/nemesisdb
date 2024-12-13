import unittest
from base import SortedIntArrayTest


class Clear(SortedIntArrayTest):

  async def test_clear(self):
    data = [100,200,300,400,500]
    
    await self.arrays.create('a', 5)
    await self.arrays.set_rng('a', data)

    # clear 200,300,400
    await self.arrays.clear('a', 1, 4)

    values = await self.arrays.get_rng('a', 0)
    self.assertListEqual(values, [100,500])


  async def test_clear_and_set(self):
    data = [100,200,300,400,500]
    
    await self.arrays.create('a', 5)
    await self.arrays.set_rng('a', data)

    # clear 200,300,400
    await self.arrays.clear('a', 1, 4)

    # with 200,300,400 cleared, add different ints
    await self.arrays.set_rng('a', [50, 250, 600])
    
    values = await self.arrays.get_rng('a', 0)
    self.assertListEqual(values, [50,100,250,500,600])


if __name__ == "__main__":
  unittest.main()