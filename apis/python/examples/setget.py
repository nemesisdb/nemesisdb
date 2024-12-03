import asyncio as asio
import sys
sys.path.append('../')
from ndb.client import NdbClient



async def setget_basics():
  client = NdbClient(debug=False) # toggle for debug

  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return

  # get single key
  await client.kv_set({'username':'billy', 'password':'billy_passy'})
  value = await client.kv_get(key='username')
  print (value)

  # get multiple keys
  await client.kv_set({'username':'billy', 'password':'billy_passy'})
  values = await client.kv_get(keys=('username', 'password'))
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

  await client.kv_set(data)
  value = await client.kv_get(key='server_users')
  print(value)

  await client.kv_set(data)
  values = await client.kv_get(keys=('server_users', 'server_port'))
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
  values = await client.kv_get(keys=('server_users', 'server_port'))
  
  print(f'Initial: {values}')
  
  # update and call set() to overwrite
  values['server_port'] = 7891
  values['server_users']['banned'] = []
  
  await client.kv_set(values)
  
  values = await client.kv_get(keys=('server_users', 'server_port'))
  print(f'Updated: {values}')


async def add():
  client = NdbClient()

  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return

  await client.kv_set({'LinuxDistro':'Arch'})
  value = await client.kv_get(key='LinuxDistro')
  print(f'Before add(): {value}')

  # kv_add() does not overwrite
  await client.kv_add({'LinuxDistro':'Arch btw'})
  value = await client.kv_get(key='LinuxDistro')
  print(f'After add(): {value}')

  # kv_set() does overwrite
  await client.kv_set({'LinuxDistro':'Arch btw'})
  value = await client.kv_get(key='LinuxDistro')
  print(f'After set(): {value}')



if __name__ == "__main__":
  for f in [setget_basics(), setget_objects(), setget_overwrite(), add()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)