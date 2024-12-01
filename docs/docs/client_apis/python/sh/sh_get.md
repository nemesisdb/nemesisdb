---
sidebar_position: 30
displayed_sidebar: clientApisSidebar
sidebar_label: sh_get
---

# sh_get
Retrieves keys from a session.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|tuple|Tuple of keys to retrieve|Y|
|tkn|int|Session token|Y|


## Returns
`dict` - key/value pairs


## Examples

```py
client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

# create session, which returns a Session, containing the token (tkn)
session = await client.sh_create_session()
# set keys, username and password
await client.sh_set({'username':'billy', 'password':'billy_passy'}, session.tkn)
# retrieve the keys
values = await client.sh_get(('username','password'), session.tkn)
print(values)

# delete the session when finished
# if the session was created with expiry settings, it will delete when it expires
await client.sh_end(session.tkn)
```