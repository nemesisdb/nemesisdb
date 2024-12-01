import unittest
from base import NDBTest


class Info(NDBTest):

  async def test_info(self):
    info = await self.client.server_info()
    self.assertTrue('serverVersion' in info)
    self.assertEqual(info['persistEnabled'], True)


if __name__ == "__main__":
  unittest.main()