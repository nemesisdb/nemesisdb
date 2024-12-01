---
sidebar_position: 2
displayed_sidebar: clientApisSidebar
sidebar_label: kv_set
---

# kv_set
Store keys to the database.

Existing keys are overwritten, to avoid this use [kv_add](./Add).

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|dict|A dictionary of key/values to store|Y|


## Returns
None


## Examples


__KV__
```py title='Set scalar'
from ndb.client import NdbClient


client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

await client.kv_set({'username':'billy', 'password':'billy_passy'})
values = await client.kv_get(('username',))
print(values)
  
```

<br/>

```py title='Set object'
from ndb.client import NdbClient


client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

data = {  "server_ip":"123.456.7.8",
          "server_port":1987,
          "server_users":
          {
            "admins":["user1", "user2"],
            "banned":["user3"]
          }
        }

await client.kv_set(data)
values = await client.kv_get(('server_users',))
print(values)
```

In this example, `server_users` is the key with the value being an object, so `print(values)` produces:

```
'server_users':{'admins':['user1', 'user2'],'banned':['user3']}
```

See [kv_get](./Get).


