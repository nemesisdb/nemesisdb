import unittest
from base import ObjListTest


class ListLen(ObjListTest):

  async def test_empty(self):
    name = 'list'

    await self.lists.create(name)
    length = await self.lists.length(name)  
    self.assertEqual(0, length)


  async def test_not_empty(self):
    name = 'list'

    await self.lists.create(name)
    await self.lists.add(name, [{'a':0}, {'b':0}, {'c':0}])
    
    length = await self.lists.length(name)  
    self.assertEqual(3, length)
  

if __name__ == "__main__":
  unittest.main()