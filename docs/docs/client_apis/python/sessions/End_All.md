---
sidebar_position: 3
displayed_sidebar: clientApisSidebar
sidebar_label: end_all_sessions
---

# end_all_sessions
Ends all sessions.

The sessions is ended immediately, with all keys being deleted, effectively clearing the database.


## Returns

`tuple(bool, int)`
- `bool` : `True` is the command was successful, otherwise `False`
- `int` : the number of sessions ended


## Examples


```py title='Create and end multiple sessions'
from ndb.sessionclient import SessionClient


client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

session1 = await client.create_session()
session2 = await client.create_session()

if session1.isValid and session2.isValid:
  print(f'Session 1 token {session1.token}')
  print(f'Session 2 token {session2.token}')

(ok, count) = await client.end_all_sessions()
if ok:
  print(f'Ended {count} sessions')
```
