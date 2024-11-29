---
sidebar_position: 1
displayed_sidebar: clientApisSidebar
---

# Quick Start

Keys can be stored independently with a `KvClient` or in a session with `SessionClient`.

`SessionClient` supports less commands, but both are similar:

- import either:
  - `from ndb.kvclient import KvClient`
  - `from ndb.sessionclient import SessionClient`
- Call `open()` to connect to the server
- Use functions such as `set()` and `get()`
- Functions raise an exception if the query fails


```py
client = KvClient()
await client.open('ws://127.0.0.1:1987/')

await client.set({'username':'billy', 'password':'billy_passy'})

values = await client.get(('username','password'))
print(values)
```

## Sessions

- Create a session with `SessionClient.create_session()`
- This returns a `Session` containing the token, which uniquely identifies the session
- The `SessionClient` commands functions require a token (`tkn`).


```py
client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.create_session()
assert session.isValid

print(f"Session created with token: {session.tkn}")

# set keys in the session
await client.set({'fname':'James', 'sname':'smith'}, session.tkn)

# retrieve
values = await client.get(('fname', 'sname'), session.tkn)
print(values)
```