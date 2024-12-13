import asyncio as asio
import sys
sys.path.append('../')

from ndb.client import NdbClient, Session


async def basics():
  client = NdbClient(debug=False) # toggle for debug logs
  try:
    await client.open('ws://127.0.0.1:1987/')
  except:
    print('Failed to connect')
    return

  session = await client.sh_create()
  if not session.tknValid:
    return

  print(f"Session created with session token: {session.tkn}")

  # set keys in the session
  await client.sh_set(session.tkn, {'fname':'James', 'sname':'smith'})


  # retrieve
  values = await client.sh_get(session.tkn, keys=('fname', 'sname'))
  print(values)
  

  # overwrite surname ('smith' to 'Smith')
  await client.sh_set(session.tkn, {'sname':'Smith'})

  # retrieve updated value
  values = await client.sh_get(session.tkn, keys=('fname', 'sname'))
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
    
    await client.sh_set(tkn, user)
  

  async def updatePassword(tkn: int):
    # we don't need username, because the session token uniquely identifies this user's data
    await client.sh_set(tkn, {'reset_password':False})


  async def failedAuth(tkn: int):
    await client.sh_set(tkn, {'reset_password':True, 'blocked':True})


  async def updateRoles(tkn: int):
    # overwrite 'roles'
    await client.sh_set(tkn, {'roles':['StandardUser']})


  client = NdbClient()
  try:
    await client.open('ws://127.0.0.1:1987/')
  except:
    print('Failed to connect')
    return

  session_user1 = await client.sh_create()
  session_user2 = await client.sh_create()

  print(f'User1 Session: {session_user1.tkn}\nUser2 Session: {session_user2.tkn}')

  await newUser(session_user1.tkn, 'user1')
  await newUser(session_user2.tkn, 'user2')

  # user1 resets their password
  await updatePassword(session_user1.tkn)
    
  # user1 account changed, update their roles
  await updateRoles(session_user1.tkn)

  # user2 fails auth so must be blocked
  await failedAuth(session_user2.tkn)


if __name__ == "__main__":
  for f in [basics(), multiple_sessions()]:
    print(f'---- {f.__name__} ----')
    asio.run(f)

