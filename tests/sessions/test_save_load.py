import unittest
import random
import os
from base import NDBSessionTest


class SaveLoad(NDBSessionTest):

  """
  Save/Load test can be skipped by calling with  './run.sh skip' which sets NDB_SKIP_SAVELOAD
  """

  @unittest.skipIf(os.environ.get('NDB_SKIP_SAVELOAD') != None, 'Skipping Save/Load')
  async def test_save_load(self):
    nSessions = 10
    tokens = []

    for _ in range(0, nSessions):
      session = await self.client.create_session()
      self.assertTrue(session.isValid)
      
      await self.client.set({'a':10, 'b':'x'}, session.tkn)
      tokens.append(session.tkn)
    

    # save
    datasetName = 'session_'+ str(random.randint(1000,999999))
    await self.client.save(datasetName)

    # end all
    clearedCount = await self.client.end_all_sessions()
    self.assertEqual(clearedCount, nSessions)

    # load
    loadedInfo = await self.client.load(datasetName)
    self.assertDictEqual(loadedInfo, {'sessions':nSessions, 'keys':nSessions*2, 'name':datasetName})  # we set 2 keys per sessions


if __name__ == "__main__":
  unittest.main()