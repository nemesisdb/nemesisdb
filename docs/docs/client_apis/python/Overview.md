---
sidebar_position: 1
displayed_sidebar: clientApisSidebar
---

# Quick Start

The `NdbClient` is the entry point:
- `from ndb.client import NdbClient`
- `open()` and `close()` for the connection
- Commands are sent with functions beginning:
  - `kv_` for keys not in a session, i.e. `kv_set()`
  - `sh_` for keys in a session, i.e. `sh_get()`, `sh_create_session()`, etc
- If a command returns an failure, an `ndb.ResponseError` is raised
  - The exception message contains the error code
  - The exception contains `rsp` which is the full response

<br/>

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

await client.kv_set({'username':'billy', 'password':'billy_passy'})

values = await client.kv_get(('username','password'))
print(values)
```

<br/>

## Sessions

Use the `sh_` functions:

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()
assert session.isValid

print(f"Session created with token: {session.tkn}")

# set keys in the session
await client.sh_set({'fname':'James', 'sname':'smith'}, session.tkn)

# retrieve
values = await client.sh_get(('fname', 'sname'), session.tkn)
print(values)
```