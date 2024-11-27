import unittest
from base import NDBSessionTest


class Info(NDBSessionTest):

  async def test_info(self):
    session = await self.client.create_session()
    self.assertTrue(session.isValid)
    
    set = await self.client.set({'a':10, 'b':'x'}, session.tkn)
    self.assertTrue(set)

    (valid, info) = await self.client.session_info(session.tkn)
    self.assertTrue(session.isValid)
    self.assertEqual(info['tkn'], session.tkn)
    self.assertEqual(info['keyCnt'], 2)
    # session does not expire, so 'expires' is False and no 'expiry' info
    self.assertEqual(info['expires'], False)
    self.assertEqual('expiry' in info, False)


  async def test_info_all(self):
    nSessions = 10
    tokens = []

    for _ in range(0, nSessions):
      session = await self.client.create_session()
      self.assertTrue(session.isValid)
      
      set = await self.client.set({'a':10, 'b':'x'}, session.tkn)
      self.assertTrue(set)

      tokens.append(session.tkn)


    (valid, info) = await self.client.session_info_all()    
    self.assertTrue(valid)
    self.assertEqual(info['totalSessions'], nSessions)
    self.assertEqual(info['totalKeys'], nSessions*2)  # we set 2 keys per session
    


if __name__ == "__main__":
  unittest.main()