import unittest
from base import NDBTest


class Info(NDBTest):

  async def test_info(self):
    input = {'i':100}

    (valid, info) = await self.client.server_info()
    self.assertTrue(valid)
    self.assertTrue('serverVersion' in info)
    self.assertEqual(info['sessionsEnabled'], False)
    self.assertEqual(info['persistEnabled'], True)


if __name__ == "__main__":
  unittest.main()