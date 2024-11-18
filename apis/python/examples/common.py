import sys
import asyncio as asio

sys.path.append('../')  # TODO this is probably wrong
from ndb.kvclient import KvClient


async def connect_client(client: KvClient, uri = 'ws://127.0.0.1:1987/'):
  return await client.listen(uri=uri)
  

async def connect(uri = 'ws://127.0.0.1:1987/'):
  client = KvClient()
  return await connect_client(client, uri)


def stop(client: KvClient):
  client.get_listen_task().cancel()
