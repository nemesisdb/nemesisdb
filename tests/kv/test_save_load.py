import random
import unittest
import os
from common import NDBTest


class SaveLoad(NDBTest):
  """
  Save load requires the server has persistance enabled. It is not possible to find this
  via a command, so it can be skipped by calling with  './run.sh skip' which sets
  NDB_SKIP_SAVELOAD
  """

  @unittest.skipIf(os.environ.get('NDB_SKIP_SAVELOAD') != None, 'Skipping Save/Load')
  async def test_save_load(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    valid = await self.client.set(input)
    self.assertTrue(valid)

    datasetName = 'test_'+ str(random.randint(1000,999999))
    
    saved = await self.client.save(datasetName)
    self.assertTrue(saved)

    (cleared, clearedCount) = await self.client.clear()
    self.assertTrue(cleared)
    self.assertEqual(clearedCount, len(input))

    # load
    (loaded, loadedCount) = await self.client.load(datasetName)
    self.assertTrue(loaded)
    self.assertEqual(loadedCount, len(input))



if __name__ == "__main__":
  unittest.main()