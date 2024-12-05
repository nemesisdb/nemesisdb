---
sidebar_position: 3
displayed_sidebar: clientApisSidebar
sidebar_label: sh_end_all
---

# sh_end_all
Ends all sessions.

Sessions with expiry settings are also ended immediately.


```py
sh_end_all() -> int
```


## Examples


```py title='Create and end multiple sessions'
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session1 = await client.sh_create()
session2 = await client.sh_create()

if session1.isValid and session2.isValid:
  print(f'Session 1 token {session1.tkn}')
  print(f'Session 2 token {session2.tkn}')

count = await client.sh_end_all()
print(f'Ended {count} sessions')
```
