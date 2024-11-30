import sys
sys.path.append('../')

from ndb.sessionclient import SessionClient, Session

import asyncio as asio



async def basics():
  client = SessionClient(debug=False) # toggle for debug logs
  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return

  session = await client.create_session()
  if session == None:
    return

  print(f"Session created with session token: {session.tkn}")

  # set keys in the session
  await client.set({'fname':'James', 'sname':'smith'}, session.tkn)


  # retrieve
  values = await client.get(('fname', 'sname'), session.tkn)
  print(values)
  

  # overwrite surname ('smith' to 'Smith')
  await client.set({'sname':'Smith'}, session.tkn)

  # retrieve updated value
  values = await client.get(('fname', 'sname'), session.tkn)
  print(values)



"""Each session has a dedicated map. So we can manipulate data
in different sessions by using the appropriate session token.
""" 
async def multiple_sessions():
  
  async def newUser(tkn: int, uname: str):
    # A new user is prompted to reset password on first login and has limited roles
    user = {'username':uname,
            'reset_password':True,
            'blocked':False,
            'roles':['NewUser', 'ReadOnly']}    
    
    await client.set(user, tkn)
  

  async def updatePassword(tkn: int):
    # we don't need username, because the session token uniquely identifies this user's data
    await client.set({'reset_password':False}, tkn)


  async def failedAuth(tkn: int):
    await client.set({'reset_password':True, 'blocked':True}, tkn)


  async def updateRoles(tkn: int):
    # overwrite 'roles'
    await client.set({'roles':['StandardUser']}, tkn)


  client = SessionClient()
  if not (await client.open('ws://127.0.0.1:1987/')):
    print('Failed to connect')
    return

  session_user1 = await client.create_session()
  session_user2 = await client.create_session()

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
  for f in [basics(), multiple_sessions(), two_sessions()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)

