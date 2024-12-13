import asyncio as asio
import sys
sys.path.append('../')

from ndb.client import NdbClient, Session


# code below used in the API README

async def newUser(client: NdbClient, username: str, email: str) -> Session:
  session = await client.sh_create()
  await client.sh_set(session.tkn, {'username':username, 'email':email})
  return session


async def updateEmail(client: NdbClient, email: str, tkn: int):
  await client.sh_set(tkn, {'email':email})


async def updateUsername(client: NdbClient, username: str, tkn: int):
  await client.sh_set(tkn, {'username':username})


async def getUser(client: NdbClient, tkn: int):
  values = await client.sh_get(tkn, keys=('username', 'email'))
  print(values)


async def two_sessions():
  client = NdbClient()
  try:
    await client.open('ws://127.0.0.1:1987/')
  except:
    print('Failed to connect')
    return
  
  user1_session = await newUser(client, 'user1', 'u1@mailg.com')
  user2_session = await newUser(client, 'user2', 'u2@yoohaa.com')

  await getUser(client, user1_session.tkn)
  await getUser(client, user2_session.tkn)

  print("Updating")
  await updateEmail(client, 'u1@chimpmail.com', user1_session.tkn)
  await updateUsername(client, 'user2_updated', user2_session.tkn)

  await getUser(client, user1_session.tkn)
  await getUser(client, user2_session.tkn)


if __name__ == "__main__":
    asio.run(two_sessions())

