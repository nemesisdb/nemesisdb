import unittest
from base import NDBSessionTest


class NewEnd(NDBSessionTest):

  async def test_create_single(self):
    session = await self.client.create_session()
    self.assertTrue(session.isValid)

  
  async def test_create_multiple(self):
    nSessions = 10

    for _ in range(0, nSessions):
      session = await self.client.create_session()
      self.assertTrue(session.isValid)


  async def test_end_single(self):
    session = await self.client.create_session()
    self.assertTrue(session.isValid)

    await self.client.end_session(session.tkn)

  
  async def test_end_multiple(self):
    nSessions = 10
    sessions = []

    for _ in range(0, nSessions):
      session = await self.client.create_session()
      self.assertTrue(session.isValid)
      sessions.append(session)

    for session in sessions:
      await self.client.end_session(session.tkn)


if __name__ == "__main__":
  unittest.main()