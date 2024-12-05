---
sidebar_position: 0
displayed_sidebar: clientApisSidebar
---

# Quick Start

The `NdbClient` is the entry point:
- `from ndb.client import NdbClient, Session`
- `open()` to connect
- Functions starting `sh_` to manage sessions and their keys
- `sh_create()` creates a new session, returning a `Session` object
- `sh_end()` ends a session
- `sh_set()`, `sh_get()`, etc to store/access keys
- If a command returns an failure, an `ndb.ResponseError` is raised
  - The exception message contains the error code
  - The exception contains `rsp` which is the full response

<br/>

```python title='Create session and set/get keys'
from ndb.client import NdbClient, Session

client = NdbClient(debug=False) # toggle for debug logs
if not (await client.open('ws://127.0.0.1:1987/')):
  print('Failed to connect')
  return

session = await client.sh_create()
if not session.tknValid:
  return

print(f"Session created with session token: {session.tkn}")

# set keys in the session
await client.sh_set(session.tkn, {'fname':'James', 'sname':'smith'})

# retrieve from session
values = await client.sh_get(session.tkn, keys=('fname', 'sname'))
print(values)


# overwrite surname ('smith' to 'Smith')
await client.sh_set(session.tkn, {'sname':'Smith'})

# retrieve updated value
values = await client.sh_get(session.tkn, keys=('fname', 'sname'))
print(values)
```
<br/>

The next example shows how you can use a session to represent user data:

- Create a separate session for each user
- `newUser()` sets the defaults
- The other functions update those values as required

Using sessions like this is a convenient way to separate data, switching between
users by using the appropriate token.

```python title='Create multiple sessions'
from ndb.client import NdbClient, Session

async def newUser(tkn: int, uname: str):
  # set defaults for the user
  user = {
           'username':uname,
           'reset_password':True,
           'blocked':False,
           'roles':['NewUser', 'ReadOnly']
         }    
  
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
if not (await client.open('ws://127.0.0.1:1987/')):
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
```
