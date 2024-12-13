import unittest
from base import SessionTest


class NewEnd(SessionTest):

  async def test_create_single(self):
    session = await self.client.sh_create()
    self.assertTrue(session.tknValid)

  
  async def test_create_multiple(self):
    nSessions = 10

    for _ in range(0, nSessions):
      session = await self.client.sh_create()
      self.assertTrue(session.tknValid)


  async def test_end_single(self):
    session = await self.client.sh_create()
    self.assertTrue(session.tknValid)

    await self.client.sh_end(session.tkn)

  
  async def test_end_multiple(self):
    nSessions = 10
    sessions = []

    for _ in range(0, nSessions):
      session = await self.client.sh_create()
      self.assertTrue(session.tknValid)
      sessions.append(session)

    for session in sessions:
      await self.client.sh_end(session.tkn)


  # expiry
  async def test_create_expires(self):
    session = await self.client.sh_create(durationSeconds=5)
    self.assertTrue(session.tknValid)


  # error conditions
  async def test_err_duration(self):
    with self.assertRaises(ValueError):
      await self.client.sh_create(durationSeconds=-1)


  async def test_err_incorrect_params(self):
    """ error because durationSeconds=0 means never expire,
        so extendOnGet, extendOnSetAdd and deleteSession have no affect
    """
    with self.assertRaises(ValueError):
      await self.client.sh_create(durationSeconds=0, extendOnGet=True)
      


if __name__ == "__main__":
  unittest.main()