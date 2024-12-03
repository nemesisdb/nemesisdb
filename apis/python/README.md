# Python API


> [!WARNING]
> This API is not production ready. I am not an experienced Python dev, so if you see improvements please do submit. This is a starting point for future work.


The API is implemented with the [websockets](https://websockets.readthedocs.io/en/stable/) library which uses asyncio.

See the NemesisDB [docs](https://docs.nemesisdb.io/client_apis/Overview).

Functions for all **common** commands are provided.

Unsupported:
- KV_SETQ
- KV_ADDQ


<br/>

# Quick Start

The `NdbClient` class handles the connection and commands.


## Set/Get Scalar
Set then get `username` and `password` keys:

```py
from ndb.client import NdbClient

client = NdbClient(debug=False) # toggle for debug

if not (await client.open('ws://127.0.0.1:1987/')):
  print('Failed to connect')
  return

# get single key
await client.kv_set({'username':'billy', 'password':'billy_passy'})
value = await client.kv_get(key='username')
print (value)

# get multiple keys
await client.kv_set({'username':'billy', 'password':'billy_passy'})
values = await client.kv_get(keys=('username', 'password'))
print (values)
```

Output:
```python
billy
{'password': 'billy_passy', 'username': 'billy'}
```

- `set()` accepts a `dict` of key:value
- `get()` accepts a `keys` tuple or `key` string, returning
  - With `keys`: `dict` of keys/values
  - With `key`: the value


<br/>

## Set/Get Objects
A key's value can be an object, as here with `server_users`:

```json
{
  "server_ip":"123.456.7.8",
  "server_port":1987,
  "server_users":
  {
    "admins":["user1", "user2"],
    "banned":["user3"]
  }
}
```

Set the data then get `server_users`:

```py
from ndb.client import NdbClient

client = NdbClient()

if not (await client.open('ws://127.0.0.1:1987/')):
  print('Failed to connect')
  return

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

await client.kv_set(data)
values = await client.kv_get(keys=('server_users', 'server_port'))
print(values)
```

Output:

```python
{'admins': ['user1', 'user2'], 'banned': ['user3']}
{'server_port': 1987, 'server_users': {'admins': ['user1', 'user2'], 'banned': ['user3']}}
```

<br/>

## Commands
Common commands:

|Command|Purpose|
|---|---|
|`kv_rmv()`|Deletes keys|
|`kv_clear()`|Deletes all keys|
|`kv_clear_set()`|Deletes all keys then sets new keys in a single command|
|`kv_count()`|Returns the number of keys|
|`kv_contains()`|Given an array of keys, returns those that exist|
|`kv_keys`|Returns all key names (i.e. no values)|


<br/>

# Sessions
A session is used to group related keys (like a Redis hashset or a class object in an OO programming):

- contains a group of keys
- has a dedicated map
- can expire, deleting keys, and optionally also deleting the session

There are functions to manage sessions and keys, beginning with `sh_`, such as `sh_create_session()` `sh_set()`, `sh_get()`. 

<br/>

A session is identified by a unique session token (64-bit integer), created by `sh_create_session()`:

```py3
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()
if not session.isValid:
  return

print(f"Session created with token: {session.tkn}")

await client.sh_set(session.tkn, {'fname':'James', 'sname':'smith'})
```

Output

```
Session created with session token: 16204359010587816757
```

- `create_session()` creates the session, returning a `Session` object
- The `Session` object contains the session token (`tkn`)
- Then use `sh_set()`, passing the token

