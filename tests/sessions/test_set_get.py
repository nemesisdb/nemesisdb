import unittest
from base import NDBSessionTest


class SetGet(NDBSessionTest):

  async def test_set(self):
    session = await self.client.sh_create()
    self.assertTrue(session.tknValid)
    
    await self.client.sh_set(session.tkn, {'a':10, 'b':'x'},)
    
    info = await self.client.sh_info(session.tkn)
    self.assertTrue(session.tknValid)
    self.assertEqual(info['tkn'], session.tkn)
    self.assertEqual(info['keyCnt'], 2)
    # session does not expire, so 'expires' is False and no 'expiry' info
    self.assertEqual(info['expires'], False)
    self.assertEqual('expiry' in info, False)


  async def test_info_all(self):
    nSessions = 10
    tokens = []

    for _ in range(0, nSessions):
      session = await self.client.sh_create()
      self.assertTrue(session.tknValid)
      
      await self.client.sh_set(session.tkn, {'a':10, 'b':'x'})

      tokens.append(session.tkn)


    info = await self.client.sh_info_all()    
    self.assertEqual(info['totalSessions'], nSessions)
    self.assertEqual(info['totalKeys'], nSessions*2)  # we set 2 keys per session
    


if __name__ == "__main__":
  unittest.main()