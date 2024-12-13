import unittest
from base import SessionTest


class Info(SessionTest):

  async def test_info(self):
    session = await self.client.sh_create()
    self.assertTrue(session.tknValid)
    
    await self.client.sh_set(session.tkn, {'a':10, 'b':'x'})
    
    info = await self.client.sh_info(session.tkn)
    self.assertEqual(info['tkn'], session.tkn)
    self.assertEqual(info['keyCnt'], 2)
    # session does not expire, so 'expires' is False and no 'expiry' info
    self.assertFalse(info['expires'])
    self.assertFalse('expiry' in info)


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
    

  async def test_info_expiry_1(self):
    session = await self.client.sh_create(durationSeconds=10, extendOnGet=True)
    self.assertTrue(session.tknValid)

    info = await self.client.sh_info(session.tkn)

    self.assertTrue(info['expires'])
    self.assertTrue('expiry' in info)
    self.assertEqual(info['expiry']['duration'], 10)
    self.assertTrue(info['expiry']['extendOnGet'])
    self.assertFalse(info['expiry']['extendOnSetAdd'])


  async def test_info_expiry_2(self):
    session = await self.client.sh_create(durationSeconds=10, extendOnSetAdd=True)
    self.assertTrue(session.tknValid)

    info = await self.client.sh_info(session.tkn)
    
    self.assertTrue(info['expires'])
    self.assertTrue('expiry' in info)
    self.assertEqual(info['expiry']['duration'], 10)
    self.assertFalse(info['expiry']['extendOnGet'])
    self.assertTrue(info['expiry']['extendOnSetAdd'])


if __name__ == "__main__":
  unittest.main()