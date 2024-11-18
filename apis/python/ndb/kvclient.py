from ndb.client import Client, FieldValues, Fields, KvCmd
from typing import Tuple


"""Client for when NemesisDB has sessions disabled.
"""
class KvClient(Client):
  # no extra work required, just use Client functions as they are.
  def __init__(self):
    super().__init__()

