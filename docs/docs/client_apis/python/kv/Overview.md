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
- If a command returns an failure, an `ndb.ResponseError` is raised
  - The exception message contains the error code
  - The exception contains `rsp` which is the full response

<br/>

```py
from ndb.client import NdbClient

client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

await client.kv_set({'username':'billy', 'password':'billy_passy'})

values = await client.kv_get(('username','password'))
print(values)
```
