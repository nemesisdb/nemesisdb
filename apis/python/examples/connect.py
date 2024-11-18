import common
import asyncio as asio
from ndb.kvclient import KvClient


async def connect_close():
  client = KvClient()
  await client.listen('ws://127.0.0.1:1987/')

  print('Connected')

  # call query functions

  await client.close()  

  print('Disconnected')


async def reconnect():
  client = KvClient()
  await client.listen('ws://127.0.0.1:1987/')
  print('Connected')
  await client.close()
  print('Disconnected')

  await client.listen('ws://127.0.0.1:1987/')
  print('Connected')
  await client.close()
  print('Disconnected')



async def multiple_clients():

  async def create() -> KvClient:
    client = KvClient()
    await client.listen('ws://127.0.0.1:1987/')
    return client


  client1 = await create()
  client2 = await create()

  await client1.set({'c1_k':'c1_v'})
  await client2.set({'c2_k':'c2_v'})

  await client1.close()

  (ok, values) = await client2.get(('c1_k', 'c2_k'))
  assert 'c1_k' in values and 'c2_k' in values


if __name__ == "__main__":
  for f in [connect_close(), reconnect(), multiple_clients()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)