import common
from common import connect_client, stop
import asyncio as asio
from ndb import Client


async def setget_basics():
  client = Client()

  listen_task = await connect_client(client)

  setSuccess = await client.set({'username':'billy', 'password':'billy_passy'})

  if setSuccess:
    (getOk, values) = await client.get(['username'])
    if getOk:
      print(values)
    else:
      print('Query failed')

  stop(client)


async def setget_objects():
  client = Client()

  listen_task = await connect_client(client)

  data = {  "server_ip":"123.456.7.8",
            "server_port":1987,
            "server_users":
            {
              "admins":["user1", "user2"],
              "banned":["user3"]
            }
          }

  setSuccess = await client.set(data)
  if setSuccess:
    (getOk, values) = await client.get(['server_users'])
    if getOk:
      print(values)

  stop(client)


if __name__ == "__main__":
  for f in [setget_basics(), setget_objects()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)