import sys
import asyncio as asio

sys.path.append('../')
from ndb import Client


async def connect_client(client: Client, uri = 'ws://127.0.0.1:1987/'):
  return await client.listen(uri=uri)
  

async def connect(uri = 'ws://127.0.0.1:1987/'):
  client = Client()
  return await connect_client(client, uri)


def stop(client: Client):
  client.get_listen_task().cancel()
