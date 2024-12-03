---
sidebar_position: 2
displayed_sidebar: clientApisSidebar
sidebar_label: kv_set
---

# kv_set
Store keys to the database.

Existing keys are overwritten, to avoid this use [kv_add](./Add).

```py
kv_set(keys: dict)
```

|Param|Description|
|--|--|
|keys|Key/values to store|


## Returns
`None`

## Raises
- `ResponseError` if query fails


## Examples

```py title='Connect'
from ndb.client import NdbClient

client = NdbClient(debug=False) # toggle for debug
await client.open('ws://127.0.0.1:1987/')
```


```py title='Set scalar'

await client.kv_set({'username':'billy', 'password':'billy_passy'})

value = await client.kv_get(key='username')
print (value)

# get multiple keys, returns a dict of key:value
values = await client.kv_get(keys=('username', 'password'))
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

await client.kv_set(data)
value = await client.kv_get(key='server_users')
print(value)

values = await client.kv_get(keys=('server_ip', 'server_port'))
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

await client.kv_set(data)
values = await client.kv_get(keys=('server_users', 'server_port'))

print(f'Initial: {values}')

# update and call set() to overwrite
values['server_port'] = 7891
values['server_users']['banned'] = []

await client.kv_set(values)

values = await client.kv_get(keys=('server_users', 'server_port'))
print(f'Updated: {values}')
```

Output:
```
Initial: {'server_port': 1987, 'server_users': {'admins': ['user1', 'user2'], 'banned': ['user3']}}
Updated: {'server_port': 7891, 'server_users': {'admins': ['user1', 'user2'], 'banned': []}}
```


See [kv_get](./Get).


