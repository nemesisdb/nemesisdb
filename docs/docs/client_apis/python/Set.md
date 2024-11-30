---
sidebar_position: 2
displayed_sidebar: clientApisSidebar
sidebar_label: set
---

# set
Store keys to the database.

Existing keys are overwritten, to avoid this use [add](./Add).

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|dict|A dictionary of key/values to store|Y|


## Returns
None


## Examples


__KV__
```py title='Set scalar'
from ndb.kvclient import KvClient


client = KvClient()
await client.open('ws://127.0.0.1:1987/')

await client.set({'username':'billy', 'password':'billy_passy'})
values = await client.get(('username',))
print(values)
  
```

<br/>

```py title='Set object'
from ndb.kvclient import KvClient


client = KvClient()
await client.open('ws://127.0.0.1:1987/')

data = {  "server_ip":"123.456.7.8",
          "server_port":1987,
          "server_users":
          {
            "admins":["user1", "user2"],
            "banned":["user3"]
          }
        }

await client.set(data)
values = await client.get(('server_users',))
print(values)
```

In this example, `server_users` is the key with the value being an object, so `print(values)` produces:

```
'server_users':{'admins':['user1', 'user2'],'banned':['user3']}
```

__Session__

```py
from ndb.sessionclient import SessionClient, Session


client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

# create session, which returns a Session, containing the token (tkn)
session = await client.create_session()
# set keys, username and password
await client.set({'username':'billy', 'password':'billy_passy'}, session.tkn)
# retrieve the keys
values = await client.get(('username','password'), session.tkn)
print(values)

# delete the session when finished
# if the session was created with expiry settings, it will delete when it expires
await client.end_session(session.tkn)
```

See [get](./Get).


