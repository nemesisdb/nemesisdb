---
sidebar_position: 20
displayed_sidebar: clientApisSidebar
---

# sh_set
Store keys in a session.

Existing keys are overwritten, to avoid this use [sh_add](./sh_add).

```py
sh_set(tkn: int, keys: dict) -> None
```


## Raises
- `ResponseError` if query fails


## Examples

```py title='Connect'
client = NdbClient(debug=False) # toggle for debug logs
if not (await client.open('ws://127.0.0.1:1987/')):
  print('Failed to connect')
  return
```
<br/>

```py title='Single session'
session = await client.sh_create()
print(f"Session created with session token: {session.tkn}")

# set keys in the session
await client.sh_set(session.tkn, {'fname':'James', 'sname':'smith'})

# retrieve
values = await client.sh_get(session.tkn, keys=('fname', 'sname'))
print(values)

# overwrite surname ('smith' to 'Smith')
await client.sh_set(session.tkn, {'sname':'Smith'})

# retrieve updated value
updatedSurname = await client.sh_get(session.tkn, key='sname')
print(updatedSurname)
```

See [sh_get](./sh_get).


