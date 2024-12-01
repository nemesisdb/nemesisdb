---
sidebar_position: 50
displayed_sidebar: clientApisSidebar
sidebar_label: sh_rmv
---

# sh_rmv
Delete particular key(s) from a session.

- To delete all keys, use [sh_clear](./sh_clear)
- You can use [sh_clear_set](./sh_clear_set.md) to clear all keys and set new keys in a single command


|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|tuple|Keys to delete|Y|
|tkn|int|Session token|Y|


## Returns
None


## Examples


```py title='Delete multiple keys'
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()
# ... set keys
await client.sh_rmv(('key1','key2','key3'), session.tkn)
```