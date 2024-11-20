# Python API


> [!WARNING]
> This API is not production ready. I am not an experienced Python dev, so if you see improvements please do submit. This is a starting point for future work.


The API is implemented with the [websockets](https://websockets.readthedocs.io/en/stable/) library which uses asyncio.

See the NemesisDB [docs](https://docs.nemesisdb.io/client_apis/python/Overview).

Functions for all **common** commands are provided, except:

Coming Soon
- KV_UPDATE
- KV_FIND
- KV_ARR_APPEND
- SH_NEW_SHARED
- SH_OPEN

Unsupported
- KV_SETQ
- KV_ADDQ


<br/>

# Quick Start

The `Client` class handles the connection and contains a function per NemesisDB command.


## Set/Get Scalar
Set then get `username` and `password` keys:

```python
from ndb import Client

client = Client()
await client.listen('ws://127.0.0.1:1987/')

setSuccess = await client.set({'username':'billy', 'password':'billy_passy'})

if setSuccess:
  (getOk, values) = await client.get(('username',))
  if getOk:
    print(values)
  else:
    print('Query failed')
```

Output:
```python
{'username': 'billy'}
```

- `set()` accepts a `dict` of key:value
- `get()` accepts a tuple of keys
  - Returns a tuple: `(bool, dict)`
  - `bool`: `True` if command was successful
  - `dict`: keys with values


You can retrieve multiple keys:

```python
(getOk, values) = await client.get(('username', 'password'))
```

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

```py3
client = Client()
await client.listen('ws://127.0.0.1:1987/')

data = {  "server_ip":"123.456.7.8",
          "server_port":1987,
          "server_users":
          {
            "admins":["user1", "user2"],
            "banned":["user3"]
          }
        }

setSuccess = await client.set(data)
if setSuccess:
  (getOk, values) = await client.get(('server_users',))
  if getOk:
    print(values)
```

Output:

```python
{'server_users': {'admins': ['user1', 'user2'], 'banned': ['user3']}}
```

<br/>

## Overwrite
`KV_SET` will overwrite a key if it already exists. To avoid this use `KV_ADD` which does not overwrite an existing key.

```py3
client = Client()
await client.listen('ws://127.0.0.1:1987/')

await client.set({'LinuxDistro':'Arch'})
(getOk, values) = await client.get(('LinuxDistro',))
print(f'Before add(): {values}')

await client.add({'LinuxDistro':'Arch btw'})
(getOk, values) = await client.get(('LinuxDistro',))
print(f'After add(): {values}')


await client.set({'LinuxDistro':'Arch btw'})
(getOk, values) = await client.get(('LinuxDistro',))
print(f'After set(): {values}')
```

Output:
```
Before add(): {'LinuxDistro': 'Arch'}
After add(): {'LinuxDistro': 'Arch'}
After set(): {'LinuxDistro': 'Arch btw'}
```

<br/>

## Commands
Common commands:

|Command|Purpose|
|---|---|
|`rmv()`|Deletes keys|
|`clear()`|Deletes all keys|
|`clear_set()`|Deletes all keys then sets new keys in a single command|
|`count()`|Returns the number of keys|
|`contains()`|Given an array of keys, returns those that exist|
|`keys`|Returns all key names (i.e. no values)|


<br/>

# Sessions
A session is similar to a Redis hashset. Each session:

- contains a group of keys
- has a dedicated map
- can expire, deleting keys, and optionally also deleting the session

The session API is implemented so that the same key-value functions (i.e. `set()`, `get()`, etc) can be used, rather than a separate group of functions.

<br/>

## Basics

A session is identified by a unique session token (64-bit integer).

```py3
client = SessionClient()
await client.listen('ws://127.0.0.1:1987/')

session = await ndb.create_session(client)
if session == None:
  return

print(f"Session created with session token: {session.tkn}")

ok = await client.set({'fname':'James', 'sname':'smith'}, session.tkn)
if not ok:
  print('Set failed')
  return
```

Output

```
Session created with session token: 16204359010587816757
{'fname': 'James', 'sname': 'smith'}
```

- Create a `SessionClient`, rather than a `Client`
- `create_session()` creates the session
- The `Session` object stores the session token and client (command functions and websocket connection)
- There after we use the same functions, such as `set()`, but we pass the token


<br/>

## Multiple Sessions
Because each session has a dedicated map, it's trivial to store and handle data that represents different entities. 
If we have a session per user to store username and email:

```py3
async def newUser(client: SessionClient, username: str, email: str) -> Session:
  session = await ndb.create_session(client)
  await client.set({'username':username, 'email':email}, session.tkn)
  return session


async def updateEmail(session: Session, email: str):
  await session.client.set({'email':email}, session.tkn)


async def updateUsername(session: Session, username: str):
  await session.client.set({'username':username}, session.tkn)


async def getUser(session: Session):
  (ok, values) = await session.client.get(('username', 'email'), session.tkn)
  if ok:
    print(values)


client = SessionClient()
await client.listen('ws://127.0.0.1:1987/')

user1_session = await newUser(client, 'user1', 'u1@mailg.com')
user2_session = await newUser(client, 'user2', 'u2@yoohaa.com')

await getUser(user1_session)
await getUser(user2_session)

print("Updating")
await updateEmail(user1_session, 'u1@chimpmail.com')
await updateUsername(user2_session, 'user2_updated')

await getUser(user1_session)
await getUser(user2_session)
```

Output:

```
{'username': 'user1', 'email': 'u1@mailg.com'}
{'username': 'user2', 'email': 'u2@yoohaa.com'}
Updating
{'username': 'user1', 'email': 'u1@chimpmail.com'}
{'username': 'user2_updated', 'email': 'u2@yoohaa.com'}
```