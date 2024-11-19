---
sidebar_position: 1
displayed_sidebar: clientApisSidebar
sidebar_label: create_session
---

# create_session

package: `ndb.sessionclient`

Creates a new session.

A session is uniquely identified by a session token (typically passed around the code as `tkn`), which is a 64-bit unsigned integer.

Sessions are managed by an instance of `SessionClient` from the `ndb.sessionclient` package. A `SessionClient` is not coupled to a particular session, so the token must be passed to the command functions (`set()`, `get()`, etc).

Calling: 

```py
client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.create_session()

await client.set({'key1':'value1'}, session.tkn)
```

Sets the key in that session.

Similarly:

```py
client.get(('k1','k2'), session.tkn)
```

Gets keys from that session. This applies to all command functions when using a token.

:::info
Ensure the database has sessions enabled via the [configuration](../../../home/config) file.
:::


## Returns

`Session`
- If the command was success `Session.isValid` is `True`, otherwise `False`
- If successful, the token is set in `Session.tkn`


## Examples


```py title='Create a session'
from ndb.sessionclient import SessionClient


client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.create_session()
if session.isValid:
  print(f'Session created with token {session.token}')
else
  print('Fail')
```


```py title='Create multiple sessions'
from ndb.sessionclient import SessionClient


client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

session1 = await client.create_session()
session2 = await client.create_session()

if session1.isValid and session2.isValid:
  print(f'Session 1 token {session1.token}')
  print(f'Session 2 token {session2.token}')
else
  print('Fail')
```