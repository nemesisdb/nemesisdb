---
sidebar_position: 2
displayed_sidebar: clientApisSidebar
sidebar_label: end_session
---

# end_session
Ends a session.

The session is ended immediately, with all keys being deleted.

## Returns

`bool`
- `True` is the command was successful, otherwise `False`


## Examples


```py title='Create and end a session'
from ndb.sessionclient import SessionClient


client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.create_session()
if session.isValid:
  print(f'Session created with token {session.token}')
  await session.end_session()
```
