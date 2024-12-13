import unittest
import random
import os
from base import NDBSessionTest


class SaveLoad(NDBSessionTest):
  async def test_save_load(self):
    nSessions = 10
    tokens = []

    for _ in range(0, nSessions):
      session = await self.client.sh_create()
      self.assertTrue(session.tknValid)
      
      await self.client.sh_set(session.tkn, {'a':10, 'b':'x'})
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
      session = await self.client.sh_create()
      assert session.tknValid
      for k in range(0,nKeysPerSession):
        await self.client.sh_set(session.tkn, {f'key{k}':'some_value'})
      
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