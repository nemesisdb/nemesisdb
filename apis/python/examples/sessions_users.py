import sys
sys.path.append('../')

from ndb.sessionclient import SessionClient, Session

import asyncio as asio

# code below used in the API README

async def newUser(client: SessionClient, username: str, email: str) -> Session:
  session = await client.create_session()
  await client.set({'username':username, 'email':email}, session.tkn)
  return session


async def updateEmail(client: SessionClient, email: str, tkn: int):
  await client.set({'email':email}, tkn)


async def updateUsername(client: SessionClient, username: str, tkn: int):
  await client.set({'username':username}, tkn)


async def getUser(client: SessionClient, tkn: int):
  values = await client.get(('username', 'email'), tkn)
  print(values)


async def two_sessions():
  client = SessionClient()
  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return
  
  user1_session = await newUser(client, 'user1', 'u1@mailg.com')
  user2_session = await newUser(client, 'user2', 'u2@yoohaa.com')

  await getUser(client, user1_session.tkn)
  await getUser(client, user2_session.tkn)

  print("Updating")
  await updateEmail(client, 'u1@chimpmail.com', user1_session.tkn)
  await updateUsername(client, 'user2_updated', user1_session.tkn)

  await getUser(client, user1_session.tkn)
  await getUser(client, user2_session.tkn)


if __name__ == "__main__":
    asio.run(two_sessions())

