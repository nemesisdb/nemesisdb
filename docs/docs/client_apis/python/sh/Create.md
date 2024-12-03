---
sidebar_position: 1
displayed_sidebar: clientApisSidebar
sidebar_label: sh_create_session
---

# sh_create_session

Creates a new session.


```py
sh_create_session(durationSeconds = 0,
                  deleteSessionOnExpire = False,
                  extendOnSetAdd = False,
                  extendOnGet = False) -> Session
```


|Param|Description|
|---|---|
|durationSeconds|Time in seconds until the session expires. Default `0` (never expires)|
|deleteSession|`true`: session is deleted when it expires<br/>`false`: only the keys are deleted (default)|
|extendOnSetAdd|`true`: on each set or add, the expire time is extended by `duration`<br/>`false`: default|
|extendOnGet|`true`: on each get, the expire time is extended by `duration`<br/>`false`: default|

A session is uniquely identified by a session token (typically referred to in the API as `tkn`), which is a 64-bit unsigned integer.


```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()

await client.set(session.tkn, {'key1':'value1'})
```

Sets the key in that session.

To get key(s) from a session use `sh_get()`:

```py
client.get(session.tkn, keys=('k1','k2'))
```

<br/>

## Returns

`Session`
- If the command was success `Session.isValid` is `True`, otherwise `False`
- If successful, the token is set in `Session.tkn`

<br/>

# Raises

- `ResponseError` if the query fails
- `ValueError` if:
  1. `durationSeconds` is less than 0
  2. `durationSeconds` is 0 but `deleteSessionOnExpire`, `extendOnSetAdd` or `extendGet` are `true`.
      This is because when `durationSeconds` is 0, the session cannot expire, so these have no effect 



## Examples

The `NdbClient` is not coupled to a particular session, so the token(s) must be supplied where required.


```py title='Create a session'
from ndb.sessionclient import SessionClient


client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()
if session.isValid:
  print(f'Session created with token {session.tkn}')
else
  print('Fail')
```


```py title='Create multiple sessions'
from ndb.sessionclient import SessionClient


client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

session1 = await client.sh_create_session()
session2 = await client.sh_create_session()

if session1.isValid and session2.isValid:
  print(f'Session 1 token {session1.token}')
  print(f'Session 2 token {session2.token}')
else
  print('Fail')
```