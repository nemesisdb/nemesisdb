---
sidebar_position: 3
displayed_sidebar: clientApisSidebar
sidebar_label: get
---

# get
Retrieves keys from the database.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|tuple|Tuple of keys to retrieve|Y|


## Returns
`dict` - key/value pairs


## Examples

```py title='Set various'
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

await client.set(data):
values = await client.get(('server_users', 'server_port'))
print(values)
```

Output:
```
{'server_users': {'admins': ['user1', 'user2'], 'banned': ['user3']}, 'server_port': 1987}
```

<br/>


```py title='Overwrite'
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

await client.set(data):
values = await client.get(('server_users', 'server_port'))

print(values)

# update and call set() to overwrite
values['server_port'] = 7891
values['server_users']['banned'] = []
await client.set(values)

values = await client.get(('server_users', 'server_port'))
print(values)
```

Output:
```
Initial: {'server_users': {'admins': ['user1', 'user2'], 'banned': ['user3']}, 'server_port': 1987}
Updated: {'server_users': {'admins': ['user1', 'user2'], 'banned': []}, 'server_port': 7891}
```