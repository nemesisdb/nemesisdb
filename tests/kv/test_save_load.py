import random
import unittest
import os
from base import NDBTest


class SaveLoad(NDBTest):
  """
  Save load requires the server has persistance enabled. It is not possible to find this
  via a command, so it can be skipped by calling with  './run.sh skip' which sets
  NDB_SKIP_SAVELOAD
  """

  @unittest.skipIf(os.environ.get('NDB_SKIP_SAVELOAD') != None, 'Skipping Save/Load')
  async def test_save_load(self):
    input = {'a':0, 'b':'str', 'c':True, 'd':1.5}

    await self.client.kv_set(input)

    datasetName = 'kv_'+ str(random.randint(1000,999999))
    
    await self.client.kv_save(datasetName)

    clearedCount = await self.client.kv_clear()
    self.assertEqual(clearedCount, len(input))

    # load
    loadedCount = await self.client.kv_load(datasetName)
    self.assertEqual(loadedCount, len(input))



if __name__ == "__main__":
  unittest.main()