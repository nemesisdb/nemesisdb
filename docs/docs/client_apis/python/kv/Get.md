---
sidebar_position: 3
displayed_sidebar: clientApisSidebar
sidebar_label: kv_get
---

# kv_get
Retrieves keys from the database.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|tuple|Tuple of keys to retrieve|Y|


## Returns
`dict` - key/value pairs


## Examples

### KV
```py title='Set various'
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

await client.kv_set(data):
values = await client.kv_get(('server_users', 'server_port'))
print(values)
```

Output:
```
{'server_users': {'admins': ['user1', 'user2'], 'banned': ['user3']}, 'server_port': 1987}
```

<br/>


```py title='Overwrite'
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

await client.kv_set(data):
values = await client.kv_get(('server_users', 'server_port'))

print(values)

# update and call set() to overwrite
values['server_port'] = 7891
values['server_users']['banned'] = []
await client.kv_set(values)

values = await client.kv_get(('server_users', 'server_port'))
print(values)
```

Output:
```
Initial: {'server_users': {'admins': ['user1', 'user2'], 'banned': ['user3']}, 'server_port': 1987}
Updated: {'server_users': {'admins': ['user1', 'user2'], 'banned': []}, 'server_port': 7891}
```

### Session

```py
client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

# create session, which returns a Session, containing the token (tkn)
session = await client.sh_create_session()
# set keys, username and password
await client.sh_set({'username':'billy', 'password':'billy_passy'}, session.tkn)
# retrieve the keys
values = await client.sh_get(('username','password'), session.tkn)
print(values)

# delete the session when finished
# if the session was created with expiry settings, it will delete when it expires
await client.sh_end(session.tkn)
```