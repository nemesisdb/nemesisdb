import asyncio as asio
import sys
sys.path.append('../')
from ndb.client import NdbClient
from ndb.kv import KV



async def setget_basics():
  client = NdbClient(debug=False) # toggle for debug

  try:
    await client.open('ws://127.0.0.1:1987/')
  except:
    print('Failed to connect')
    return

  kv = KV(client)

  # get single key
  await kv.set({'username':'billy', 'password':'billy_passy'})
  value = await kv.get(key='username')
  print (value)

  # get multiple keys
  await kv.set({'username':'billy', 'password':'billy_passy'})
  values = await kv.get(keys=('username', 'password'))
  print (values)


async def setget_objects():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  data = {  "server_ip":"123.456.7.8",
            "server_port":1987,
            "server_users":
            {
              "admins":["user1", "user2"],
              "banned":["user3"]
            }
          }

  kv = KV(client)
  await kv.set(data)

  value = await kv.get(key='server_users')
  print(value)

  values = await kv.get(keys=('server_ip', 'server_port'))
  print(values)


async def setget_overwrite():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')

  kv = KV(client)

  data = {  "server_ip":"123.456.7.8",
            "server_port":1987,
            "server_users":
            {
              "admins":["user1", "user2"],
              "banned":["user3"]
            }
          }

  await kv.set(data)
  values = await kv.get(keys=('server_users', 'server_port'))
  
  print(f'Initial: {values}')
  
  # update and call set() to overwrite
  values['server_port'] = 7891
  values['server_users']['banned'] = []
  
  await kv.set(values)
  
  values = await kv.get(keys=('server_users', 'server_port'))
  print(f'Updated: {values}')


async def add():
  client = NdbClient()
  await client.open('ws://127.0.0.1:1987/')
  
  kv = KV(client)

  await kv.set({'LinuxDistro':'Arch'})
  value = await kv.get(key='LinuxDistro')
  print(f'Before add(): {value}')

  # kv_add() does not overwrite
  await kv.add({'LinuxDistro':'Arch btw'})
  value = await kv.get(key='LinuxDistro')
  print(f'After add(): {value}')

  # kv_set() does overwrite
  await kv.set({'LinuxDistro':'Arch btw'})
  value = await kv.get(key='LinuxDistro')
  print(f'After set(): {value}')



if __name__ == "__main__":
  for f in [setget_basics(), setget_objects(), setget_overwrite(), add()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)