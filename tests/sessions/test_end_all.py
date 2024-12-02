import unittest
from base import NDBSessionTest


class EndAll(NDBSessionTest):

  async def test_end_all(self):
    nSessions = 10

    for _ in range(0, nSessions):
      session = await self.client.sh_create_session()
      self.assertTrue(session.isValid)

    count = await self.client.sh_end_all()
    self.assertEqual(count, nSessions)


if __name__ == "__main__":
  unittest.main()