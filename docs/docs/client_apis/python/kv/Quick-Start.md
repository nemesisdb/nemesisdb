---
sidebar_position: 1
displayed_sidebar: clientApisSidebar
---

# Quick Start

The `NdbClient` is the entry point:
- `from ndb.client import NdbClient`
- `open()` to connect
- Functions starting `kv_` to manage key/values not in a session
- If a command returns an failure, an `ndb.ResponseError` is raised
  - The exception message contains the error code
  - The exception contains `rsp` which is the full response

<br/>

```py
from ndb.client import NdbClient

client = NdbClient(debug=False) # toggle for debug logging
await client.open('ws://127.0.0.1:1987/')

await client.kv_set({'username':'billy', 'password':'billy_passy'})

username = await client.kv_get(key='username')
print(username) # billy

values = await client.kv_get(keys=('username','password'))
print(values) # {'password':'billy_passy', 'username':'billy'}
```
