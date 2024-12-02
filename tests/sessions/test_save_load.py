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
      session = await self.client.sh_create_session()
      self.assertTrue(session.isValid)
      
      await self.client.sh_set({'a':10, 'b':'x'}, session.tkn)
      tokens.append(session.tkn)
    

    # save
    datasetName = 'session_'+ str(random.randint(1000,999999))
    await self.client.sh_save(datasetName)

    # end all
    clearedCount = await self.client.sh_end_all()
    self.assertEqual(clearedCount, nSessions)

    # load
    loadedInfo = await self.client.sh_load(datasetName)
    self.assertDictEqual(loadedInfo, {'sessions':nSessions, 'keys':nSessions*2, 'name':datasetName})  # we set 2 keys per sessions


  async def test_save_load_selected(self):
    dataSetName = f'session_{random.randint(0, 10000)}'
    nSessions = 10
    nKeysPerSession = 4
    

    # clear before start
    await self.client.sh_end_all()

    tokensToSave = list()

    # create sessions
    for s in range(0, nSessions):
      session = await self.client.sh_create_session()
      assert session.isValid
      for k in range(0,nKeysPerSession):
        await self.client.sh_set({f'key{k}':'some_value'}, session.tkn)
      
      # store every second token to save
      if s % 2 == 0:
        tokensToSave.append(session.tkn)


    await self.client.sh_save(dataSetName, tokensToSave)

    # clear and then load
    count = await self.client.sh_end_all()
    assert count == nSessions

    rsp = await self.client.sh_load(dataSetName)
    assert rsp['sessions'] == len(tokensToSave) and rsp['keys'] == len(tokensToSave)*nKeysPerSession



if __name__ == "__main__":
  unittest.main()