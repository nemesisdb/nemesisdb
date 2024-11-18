---
sidebar_position: 1
displayed_sidebar: clientApisSidebar
---

# Quick Start

The API supports sessions being enabled or disabled.

## Sessions Disabled

- Create a `ndb.Client`
- Call `Client.listen()` to connect to the server
- Use functions such as `set()` and `get()`
- Functions return either a `bool` or `tuple`:
  - If `bool`, it indicates success
  - If a `tuple`, the first value is typically a `bool` to indicate success


```py
client = Client()
client.listen('ws://127.0.0.1:1987/')

setSuccess = await client.set({'username':'billy', 'password':'billy_passy'})

if setSuccess:
  (getOk, values) = await client.get(('username',))
  if getOk:
    print(values)
  else:
    print('Query failed')
```

## Sessions Enabled

- Create a `SessionClient`
- Create a session with `ndb.create_session()`
- This returns a `Session` containing the token
- `SessionClient` inherits from `Client` so the same functions can be used but now set the token


```py
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
```