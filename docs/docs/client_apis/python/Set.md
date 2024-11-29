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


```py title='Set scalar'
client = KvClient()
await client.open('ws://127.0.0.1:1987/')

await client.set({'username':'billy', 'password':'billy_passy'})
values = await client.get(('username',))
print(values)
  
```

<br/>

```py title='Set object'
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

See [get](./Get).