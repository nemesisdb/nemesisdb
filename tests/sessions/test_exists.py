import unittest
from base import NDBSessionTest


class Exists(NDBSessionTest):
   
  async def test_exists(self):
    nSessions = 10
    tokens = []

    for _ in range(0, nSessions):
      session = await self.client.create_session()
      self.assertTrue(session.isValid)
      tokens.append(session.tkn)


    # individually
    for tkn in tokens:
      exists = await self.client.session_exists([tkn])
      self.assertListEqual([tkn], exists)


    # all
    exists = await self.client.session_exists(tokens)
    self.assertListEqual(tokens, exists)
    

if __name__ == "__main__":
  unittest.main()