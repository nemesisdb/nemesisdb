from ndb.client import Client


"""Client for when server has sessions disabled.
If sessions are enabled, use SessionClient.
"""
class KvClient(Client):
  # no extra work required, just use Client functions as they are.
  def __init__(self, debug = False):
    super().__init__(debug)

