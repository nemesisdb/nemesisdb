import unittest
from base import ObjListTest


class ListSplice(ObjListTest):

  async def test_dest_move_first_two(self):
    src = '1'
    dest = '2'
    
    await self.lists.create(src)
    await self.lists.create(dest)
    
    await self.lists.add(src, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])

    # move 'a' and 'b' to dest
    await self.lists.splice(dest, src, srcStart=0, srcEnd=2, destPos=0)

    srcValues = await self.lists.get_rng(src, start=0)
    destValues = await self.lists.get_rng(dest, start=0)

    self.assertEqual(len(srcValues), 2)
    self.assertEqual(len(destValues), 2)

    self.assertListEqual([{'c':0}, {'d':0}], srcValues)
    self.assertListEqual([{'a':0}, {'b':0}], destValues)


  async def test_dest_move_mid_two(self):
    src = '1'
    dest = '2'
    
    await self.lists.create(src)
    await self.lists.create(dest)
    
    await self.lists.add(src, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])

    # move 'b' and 'c' to dest
    await self.lists.splice(dest, src, srcStart=1, srcEnd=3, destPos=0)

    srcValues = await self.lists.get_rng(src, start=0)
    destValues = await self.lists.get_rng(dest, start=0)

    self.assertEqual(len(srcValues), 2)
    self.assertEqual(len(destValues), 2)

    self.assertListEqual([{'a':0}, {'d':0}], srcValues)
    self.assertListEqual([{'b':0}, {'c':0}], destValues)


  async def test_dest_move_last_two(self):
    src = '1'
    dest = '2'
    
    await self.lists.create(src)
    await self.lists.create(dest)
    
    await self.lists.add(src, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])

    # move 'c' and 'd' to dest
    await self.lists.splice(dest, src, srcStart=2, srcEnd=4, destPos=0)

    srcValues = await self.lists.get_rng(src, start=0)
    destValues = await self.lists.get_rng(dest, start=0)

    self.assertEqual(len(srcValues), 2)
    self.assertEqual(len(destValues), 2)

    self.assertListEqual([{'a':0}, {'b':0}], srcValues)
    self.assertListEqual([{'c':0}, {'d':0}], destValues)


  async def test_dest_move_all(self):
    src = '1'
    dest = '2'
    
    await self.lists.create(src)
    await self.lists.create(dest)
    
    await self.lists.add(src, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])
    await self.lists.splice(dest, src, srcStart=0, srcEnd=4, destPos=0)

    srcValues = await self.lists.get_rng(src, start=0)
    destValues = await self.lists.get_rng(dest, start=0)

    self.assertEqual(len(srcValues), 0)
    self.assertEqual(len(destValues), 4)

    self.assertListEqual([{'a':0}, {'b':0}, {'c':0}, {'d':0}], destValues)


  async def test_dest_not_empty(self):
    src = '1'
    dest = '2'
    
    await self.lists.create(src)
    await self.lists.create(dest)
    
    await self.lists.add(src, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])
    await self.lists.add(dest, [{'a':0}, {'d':0}])

    # move 'b' and 'c' to pos=1 of dest (so dest becomes a,b,c,d)
    await self.lists.splice(dest, src, srcStart=1, srcEnd=3, destPos=1)

    srcValues = await self.lists.get_rng(src, start=0)
    destValues = await self.lists.get_rng(dest, start=0)

    self.assertEqual(len(srcValues), 2)
    self.assertEqual(len(destValues), 4)

    self.assertListEqual([{'a':0}, {'d':0}], srcValues)
    self.assertListEqual([{'a':0}, {'b':0}, {'c':0}, {'d':0}], destValues)


  async def test_dest_not_exist(self):
    src = '1'
    dest = '2'
    
    await self.lists.create(src)
    await self.lists.add(src, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])

    # create dest then move 'a' and 'b' to dest
    await self.lists.splice(dest, src, srcStart=0, srcEnd=2, destPos=0)

    srcValues = await self.lists.get_rng(src, start=0)
    destValues = await self.lists.get_rng(dest, start=0)

    self.assertEqual(len(srcValues), 2)
    self.assertEqual(len(destValues), 2)

    self.assertListEqual([{'c':0}, {'d':0}], srcValues)
    self.assertListEqual([{'a':0}, {'b':0}], destValues)


  async def test_dest_default(self):
    src = '1'
    dest = '2'
    
    await self.lists.create(src)
    await self.lists.create(dest)

    await self.lists.add(src, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])
    await self.lists.add(dest, [{'a':0}, {'b':0}])

    # move 'c' and 'd' to end of dest
    await self.lists.splice(dest, src, srcStart=2)

    srcValues = await self.lists.get_rng(src, start=0)
    destValues = await self.lists.get_rng(dest, start=0)

    self.assertEqual(len(srcValues), 2)
    self.assertEqual(len(destValues), 4)

    self.assertListEqual([{'a':0}, {'b':0}], srcValues)
    self.assertListEqual([{'a':0}, {'b':0}, {'c':0}, {'d':0}], destValues)


  async def test_src_bounds(self):
    src = '1'
    dest = '2'
    
    await self.lists.create(src)
    await self.lists.create(dest)
    
    await self.lists.add(src, [{'a':0}, {'b':0}, {'c':0}, {'d':0}])

    # move 'b', 'c' and 'd' to dest but with srcEnd out of bounds
    await self.lists.splice(dest, src, srcStart=1, srcEnd=10, destPos=0)

    srcValues = await self.lists.get_rng(src, start=0)
    destValues = await self.lists.get_rng(dest, start=0)

    self.assertEqual(len(srcValues), 1)
    self.assertEqual(len(destValues), 3)

    self.assertListEqual([{'a':0}], srcValues)
    self.assertListEqual([{'b':0}, {'c':0},  {'d':0}], destValues)

    

if __name__ == "__main__":
  unittest.main()