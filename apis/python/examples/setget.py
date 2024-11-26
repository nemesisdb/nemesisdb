import asyncio as asio
import sys
sys.path.append('../')
from ndb.kvclient import KvClient



async def setget_basics():
  client = KvClient()

  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return

  setSuccess = await client.set({'username':'billy', 'password':'billy_passy'})

  if setSuccess:
    (getOk, values) = await client.get(('username',))
    print (values if getOk else 'Query failed')


async def setget_objects():
  client = KvClient()

  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return

  data = {  "server_ip":"123.456.7.8",
            "server_port":1987,
            "server_users":
            {
              "admins":["user1", "user2"],
              "banned":["user3"]
            }
          }

  if await client.set(data):
    (getOk, values) = await client.get(('server_users',))
    if getOk:
      print(values)


async def setget_multiple():
  client = KvClient()

  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return
  
  data = {  "server_ip":"123.456.7.8",
            "server_port":1987,
            "server_users":
            {
              "admins":["user1", "user2"],
              "banned":["user3"]
            }
          }

  if await client.set(data):
    (getOk, values) = await client.get(('server_users', 'server_port'))
  if getOk:
    print(values)


async def setget_overwrite():
  client = KvClient()

  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return

  data = {  "server_ip":"123.456.7.8",
            "server_port":1987,
            "server_users":
            {
              "admins":["user1", "user2"],
              "banned":["user3"]
            }
          }

  if await client.set(data):
    (getOk, values) = await client.get(('server_users', 'server_port'))
    if getOk:
      print(f'Initial: {values}')
      # update and call set() to overwrite
      values['server_port'] = 7891
      values['server_users']['banned'] = []
      await client.set(values)
      (getOk, values) = await client.get(('server_users', 'server_port'))
      print(f'Updated: {values}')


async def add():
  client = KvClient()

  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return

  await client.set({'LinuxDistro':'Arch'})
  (getOk, values) = await client.get(('LinuxDistro',))
  print(f'Before add(): {values}')

  await client.add({'LinuxDistro':'Arch btw'})
  (getOk, values) = await client.get(('LinuxDistro',))
  print(f'After add(): {values}')

  await client.set({'LinuxDistro':'Arch btw'})
  (getOk, values) = await client.get(('LinuxDistro',))
  print(f'After set(): {values}')



if __name__ == "__main__":
  for f in [setget_basics(), setget_objects(), setget_multiple(), setget_overwrite(), add()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)