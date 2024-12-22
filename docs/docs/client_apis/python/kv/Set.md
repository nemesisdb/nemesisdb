---
sidebar_position: 2
displayed_sidebar: clientApisSidebar
sidebar_label: set
---

# set


```py
async def set(keys: dict) -> None
```

|Param|Description|
|--|--|
|keys|Key/values to store|


Store keys to the database.

Existing keys are overwritten, to avoid this use [add](./Add).


## Raises
- `ResponseError`


## Examples

```py title='Connect and Create API'
from ndb.client import NdbClient
from ndb.kv import KV

client = NdbClient(debug=False) # toggle for debug
await client.open('ws://127.0.0.1:1987/')

kv = KV(client)
```


```py title='Set scalar'
await kv.set({'username':'billy', 'password':'billy_passy'})

value = await kv.get(key='username')
print (value)

# get multiple keys, returns a dict of key:value
values = await kv.get(keys=('username', 'password'))
print (values)
```

<br/>

```py title='Set object'
data = {  "server_ip":"123.456.7.8",
          "server_port":1987,
          "server_users":
          {
            "admins":["user1", "user2"],
            "banned":["user3"]
          }
        }

await kv.set(data)
value = await kv.get(key='server_users')
print(value)

values = await kv.get(keys=('server_ip', 'server_port'))
print(values)
```

Output:
```
{'admins': ['user1', 'user2'], 'banned': ['user3']}
{'server_ip': '123.456.7.8', 'server_port': 1987}
```

<br/>

```py title='Overwrite'
data = {
          "server_ip":"123.456.7.8",
          "server_port":1987,
          "server_users":
          {
            "admins":["user1", "user2"],
            "banned":["user3"]
          }
       }

await kv.set(data)
values = await kv.get(keys=('server_users', 'server_port'))

print(f'Initial: {values}')

# update and call set() to overwrite
values['server_port'] = 7891
values['server_users']['banned'] = []

await kv.set(values)

values = await kv.get(keys=('server_users', 'server_port'))
print(f'Updated: {values}')
```

Output:
```
Initial: {'server_port': 1987, 'server_users': {'admins': ['user1', 'user2'], 'banned': ['user3']}}
Updated: {'server_port': 7891, 'server_users': {'admins': ['user1', 'user2'], 'banned': []}}
```


See [kv_get](./Get).


