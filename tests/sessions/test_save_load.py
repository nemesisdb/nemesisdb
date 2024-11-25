import unittest
import random
import os
from base import NDBSessionTest


class SaveLoad(NDBSessionTest):

  """
  Save load requires the server has persistance enabled. It is not possible to find this
  via a command, so it can be skipped by calling with  './run.sh skip' which sets
  NDB_SKIP_SAVELOAD
  """

  @unittest.skipIf(os.environ.get('NDB_SKIP_SAVELOAD') != None, 'Skipping Save/Load')
  async def test_save_load(self):
    nSessions = 10
    tokens = []

    for _ in range(0, nSessions):
      session = await self.client.create_session()
      self.assertTrue(session.isValid)
      
      set = await self.client.set({'a':10, 'b':'x'}, session.tkn)
      self.assertTrue(set)

      tokens.append(session.tkn)
    

    # save
    datasetName = 'session_'+ str(random.randint(1000,999999))
    
    saved = await self.client.session_save(datasetName)
    self.assertTrue(saved)

    # end all
    (cleared, clearedCount) = await self.client.end_all_sessions()
    self.assertTrue(cleared)
    self.assertEqual(clearedCount, nSessions)

    # load
    (loaded, loadedInfo) = await self.client.session_load(datasetName)
    self.assertTrue(loaded)
    self.assertDictEqual(loadedInfo, {'sessions':nSessions, 'keys':nSessions*2, 'name':datasetName})  # we set 2 keys per sessions


if __name__ == "__main__":
  unittest.main()