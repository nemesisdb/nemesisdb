import asyncio as asio
import sys
sys.path.append('../')
from ndb.client import NdbClient



async def setget_basics():
  client = NdbClient(debug=False) # toggle for debug

  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return

  setSuccess = await client.kv_set({'username':'billy', 'password':'billy_passy'})

  if setSuccess:
    values = await client.kv_get(('username',))
    print (values)


async def setget_objects():
  client = NdbClient()

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

  if await client.kv_set(data):
    values = await client.kv_get(('server_users',))
    print(values)


async def setget_multiple():
  client = NdbClient()

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

  if await client.kv_set(data):
    values = await client.kv_get(('server_users', 'server_port'))
    print(values)


async def setget_overwrite():
  client = NdbClient()

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

  await client.kv_set(data)
  values = await client.kv_get(('server_users', 'server_port'))
  
  print(f'Initial: {values}')
  
  # update and call set() to overwrite
  values['server_port'] = 7891
  values['server_users']['banned'] = []
  
  await client.kv_set(values)
  
  values = await client.kv_get(('server_users', 'server_port'))
  print(f'Updated: {values}')


async def add():
  client = NdbClient()

  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return

  await client.kv_set({'LinuxDistro':'Arch'})
  values = await client.kv_get(('LinuxDistro',))
  print(f'Before add(): {values}')

  await client.kv_add({'LinuxDistro':'Arch btw'})
  values = await client.kv_get(('LinuxDistro',))
  print(f'After add(): {values}')

  await client.kv_set({'LinuxDistro':'Arch btw'})
  values = await client.kv_get(('LinuxDistro',))
  print(f'After set(): {values}')



if __name__ == "__main__":
  for f in [setget_basics(), setget_objects(), setget_multiple(), setget_overwrite(), add()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)