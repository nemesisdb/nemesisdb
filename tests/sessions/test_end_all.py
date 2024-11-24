import unittest
from common import NDBSessionTest


class EndAll(NDBSessionTest):

  async def test_end_all(self):
    nSessions = 10

    for _ in range(0, nSessions):
      session = await self.client.create_session()
      self.assertTrue(session.isValid)

    (ended, count) = await self.client.end_all_sessions()
    self.assertTrue(ended)
    self.assertEqual(count, nSessions)


if __name__ == "__main__":
  unittest.main()