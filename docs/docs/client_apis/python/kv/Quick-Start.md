---
sidebar_position: 1
displayed_sidebar: clientApisSidebar
---

# Quick Start

- `NdbClient` and `KV` are required:
  - `NdbClient` manages the connection
  - `KV` contains the key value commands
- All commands can raise a `ResponseError`:
  - The exception contains the status (`st`)
  - The exception contains `rsp` which is the full response
- Some commands can raise a `TypeError` or `ValueError`

<br/>

```py
from ndb.client import NdbClient
from ndb.kv import KV

client = NdbClient(debug=False) # toggle for debug logging

try:
  await client.open('ws://127.0.0.1:1987/')
except:
  print('Failed to connect')
  return

  
# create API object, passing client
kv = KV(client)

await kv.set({'username':'billy', 'password':'billy_passy'})

username = await kv.get(key='username')
print(username) # billy

values = await kv.get(keys=('username','password'))
print(values) # {'password':'billy_passy', 'username':'billy'}
```
