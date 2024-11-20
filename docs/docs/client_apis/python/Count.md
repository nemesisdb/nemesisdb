---
sidebar_position: 8
displayed_sidebar: clientApisSidebar
sidebar_label: count
---

# count
Returns the number of keys. 

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|dict|The new keys store after the existing keys have been deleted|Y|
|tkn|int|A session token|Only if sessions enabled|


:::note
With sessions enabled, this command only returns the number of keys in the session identified by the `tkn` parameter.
:::


## Returns

`tuple(bool, int)`
- `bool`: `True` if command successful, otherwise `False`
- `int` : the number of keys


## Examples


```py
client = KvClient()
await client.open('ws://127.0.0.1:1987/')
(success, count) = await client.count()
```
