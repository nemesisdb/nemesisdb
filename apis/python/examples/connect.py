import asyncio as asio
import sys
sys.path.append('../')
from ndb.kvclient import KvClient


async def connect_close():
  client = KvClient()
  if await client.open('ws://127.0.0.1:1987/'):
    print('Connected')

    # call query functions

    await client.close()  

    print('Disconnected')


async def reconnect():
  client = KvClient()
  if await client.open('ws://127.0.0.1:1987/'):
    print('Connected')
    await client.close()
    print('Disconnected')

    if await client.open('ws://127.0.0.1:1987/'):
      print('Connected again')
      await client.close()
      print('Disconnected')



async def multiple_clients():

  async def create() -> KvClient:
    client = KvClient()
    opened = await client.open('ws://127.0.0.1:1987/')
    return None if opened == False else client


  client1 = await create()
  client2 = await create()

  if client1 and client2:
    await client1.set({'c1_k':'c1_v'})
    await client2.set({'c2_k':'c2_v'})

    await client1.close()

    values = await client2.get(('c1_k', 'c2_k'))
    assert 'c1_k' in values and 'c2_k' in values


if __name__ == "__main__":
  for f in [connect_close(), reconnect(), multiple_clients()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)