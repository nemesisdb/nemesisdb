import unittest
from base import SvTest

class Info(SvTest):

  async def test_info(self):
    info = await self.sv.info()
    self.assertTrue('serverVersion' in info)
    self.assertTrue(info['persistEnabled'])


if __name__ == "__main__":
  unittest.main()