---
sidebar_position: 20
displayed_sidebar: clientApisSidebar
---

# sh_set
Store keys in a session.

Existing keys are overwritten, to avoid this use [sh_add](./sh_add).

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|dict|A dictionary of key/values to store|Y|
|tkn|int|Session token|Y|


## Returns
None


## Examples


```py title='Set scalar'
from ndb.client import NdbClient


client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# create session, which returns a Session, containing the token (tkn)
session = await client.sh_create_session()
# set username and password in the session
await client.sh_set({'username':'billy', 'password':'billy_passy'}, session.tkn)
# retieve keys from the session
values = await client.sh_get(('username','password'), session.tkn)
print(values)
```

<br/>

```py title='Set object'
from ndb.client import NdbClient

data = {  "server_ip":"123.456.7.8",
          "server_port":1987,
          "server_users":
          {
            "admins":["user1", "user2"],
            "banned":["user3"]
          }
        }


client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()

await client.sh_set(data, session.tkn)

# retrieve the server_users key
values = await client.sh_get(('server_users',), session.tkn)
print(values)
```

`server_users` is the key with the value being an object:

```
'server_users':{'admins':['user1', 'user2'],'banned':['user3']}
```

See [sh_get](./sh_get).


