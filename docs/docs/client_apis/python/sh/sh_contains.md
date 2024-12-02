---
sidebar_position: 90
displayed_sidebar: clientApisSidebar
sidebar_label: sh_contains
---

# sh_contains
Given a tuple of keys, returns the keys which exist in the session.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|tuple|Keys to check|Y|
|tkn|int|Session token|Y|


## Returns
`list` : the keys which exist


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()

await client.sh_set({'k1':10, 'k2':20}, session.tkn)

exist = await client.sh_contains(('k1','k2','k3'), session.tkn)
print(exist) # ['k1', 'k2']
```
