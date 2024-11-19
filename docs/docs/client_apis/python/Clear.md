---
sidebar_position: 6
displayed_sidebar: clientApisSidebar
sidebar_label: clear
---

# clear
Deletes _all_ keys.


- To delete select key(s) use [rmv](./Rmv)
- To delete all keys and set new keys in a single call, use [clear_set](./Clear_Set)


|Param|Type|Description|Required|
|--|:-:|--|:-:|
|tkn|int|A session token|Only if sessions enabled|


:::note
With sessions enabled, this command only clears keys in the session identified by the `tkn` parameter.
:::


## Returns

`bool`
- `True` if command successful, otherwise `False`


## Examples


```py
client = KvClient()
await client.open('ws://127.0.0.1:1987/')
await client.clear()
```