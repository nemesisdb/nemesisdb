import common
import asyncio as asio
from ndb import Client


async def connect_close():
  client = Client()
  listen_task = await client.listen('ws://127.0.0.1:1987/')

  print('Connected')

  # call query functions

  await client.close()  

  print('Disconnected')


async def connect_canceltask():
  client = Client()
  listen_task = await client.listen('ws://127.0.0.1:1987/')

  print('Connected')

  listen_task.cancel()

  print('Disconnected')


async def reconnect():
  
  async def connect() -> asio.Task:
    client = Client()
    listen_task = await client.listen('ws://127.0.0.1:1987/')
    print('Connected')
    return listen_task


  listen_task = await connect()
  await asio.sleep(2)

  listen_task.cancel()
  print('Disconnected')

  # connect again
  listen_task = await connect()
  await asio.sleep(2)

  listen_task.cancel()
  print('Disconnected')


if __name__ == "__main__":
  for f in [connect_close(), connect_canceltask(), reconnect()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)