---
sidebar_position: 10
displayed_sidebar: clientApisSidebar
sidebar_label: keys
---

# keys
Returns all keys, excluding values.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|tkn|int|A session token|Only if sessions enabled|


:::note
With sessions enabled, this command only returns keys in the session identified by the `tkn` parameter.
:::


## Returns

`tuple(bool, list)`
- `bool`: `True` if command successful, otherwise `False`
- `list` : the keys


## Examples


```py
client = KvClient()
await client.open('ws://127.0.0.1:1987/')
(success, allKeys) = await client.keys()
```
