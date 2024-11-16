import common
import ndb
from ndb import SessionClient, Session

import asyncio as asio



async def basics():
  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  session = await ndb.create_session(client)
  if session == None:
    return

  print(f"Session created with session token: {session.tkn}")

  # set keys in the session
  ok = await client.set({'fname':'James', 'sname':'smith'}, session.tkn)
  if not ok:
    print('Set failed')
    return


  # retrieve
  (ok, values) = await client.get(('fname', 'sname'), session.tkn)
  if not ok:
    print('Get failed')
    return

  print(values)
  

  # overwrite surname ('smith' to 'Smith')
  ok = await client.set({'sname':'Smith'}, session.tkn)
  if not ok:
    print('Set failed')

  # retrieve updated value
  (ok, values) = await client.get(('fname', 'sname'), session.tkn)
  if not ok:
    print('Get failed')

  print(values)



"""Each session has a dedicated map. So we can manipulate data
in different sessions by using the appropriate session token.
""" 
async def multiple_sessions():
  
  async def newUser(tkn: int, uname: str):
    # A new user is prompted to reset password on first login and has limited roles
    d = {'username':uname,
         'reset_password':True,
         'blocked':False,
         'roles':['NewUser', 'ReadOnly']}    
    await client.set(d, tkn)
  
  async def updatePassword(tkn: int):
    # we don't need username, because the session token uniquely identifies this user's data
    await client.set({'reset_password':False}, tkn)

  async def failedAuth(tkn: int):
    await client.set({'reset_password':True, 'blocked':True}, tkn)

  async def updateRoles(tkn: int):
    # overwrite 'roles'
    await client.set({'roles':['StandardUser']}, tkn)


  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')

  session_user1 = await ndb.create_session(client)
  session_user2 = await ndb.create_session(client)

  print(f'User1 Session: {session_user1.tkn}\nUser2 Session: {session_user2.tkn}\n')

  await newUser(session_user1.tkn, 'user1')
  await newUser(session_user2.tkn, 'user2')

  # user1 resets their password
  await updatePassword(session_user1.tkn)
  
  # user2 fails auth so must be blocked
  await failedAuth(session_user2.tkn)

  # user1 account changed, update their roles
  await updateRoles(session_user1.tkn)


# code below used in the API README

async def newUser(client: SessionClient, username: str, email: str) -> Session:
  session = await ndb.create_session(client)
  await client.set({'username':username, 'email':email}, session.tkn)
  return session


async def updateEmail(session: Session, email: str):
  await session.client.set({'email':email}, session.tkn)


async def updateUsername(session: Session, username: str):
  await session.client.set({'username':username}, session.tkn)


async def getUser(session: Session):
  (ok, values) = await session.client.get(('username', 'email'), session.tkn)
  if ok:
    print(values)


async def two_sessions():
  client = SessionClient()
  await client.listen('ws://127.0.0.1:1987/')
  
  user1_session = await newUser(client, 'user1', 'u1@mailg.com')
  user2_session = await newUser(client, 'user2', 'u2@yoohaa.com')

  await getUser(user1_session)
  await getUser(user2_session)

  print("Updating")
  await updateEmail(user1_session, 'u1@chimpmail.com')
  await updateUsername(user2_session, 'user2_updated')

  await getUser(user1_session)
  await getUser(user2_session)


if __name__ == "__main__":
  for f in [basics(), multiple_sessions(), two_sessions()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)

