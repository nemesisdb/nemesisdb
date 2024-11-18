from ndb.client import Client, FieldValues, Fields, KvCmd
from typing import Tuple


"""Client for when NemesisDB has sessions disabled.
"""
class KvClient(Client):
  def __init__(self):
    super().__init__()

