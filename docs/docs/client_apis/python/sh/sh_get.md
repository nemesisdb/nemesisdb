---
sidebar_position: 30
displayed_sidebar: clientApisSidebar
sidebar_label: sh_get
---

# sh_get
Retrieves keys from a session.

```py
sh_get(self, tkn: int, keys = (), key = None)
```
|Param|Description|
|--|--|
|key|Key to retrieve|
|keys|Keys to retrieve|


## Returns
- If `keys` set, a `dict` is returned
- Otherwise, the value of `key` is returned


## Raises
- `ResponseError` if query fails
- `ValueError` if both `keys` and `key` are set


## Examples

```py
client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

# create session, which returns a Session, containing the token (tkn)
session = await client.sh_create()
# set keys, username and password
await client.sh_set(session.tkn, {'username':'billy', 'password':'billy_passy'})

# retrieve one key
username = await client.sh_get(session.tkn, key='username')
print(username)

# retrieve several keys
values = await client.sh_get(session.tkn, keys=('username','password'))
print(values)

await client.sh_end(session.tkn)
```