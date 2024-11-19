---
sidebar_position: 9
displayed_sidebar: clientApisSidebar
sidebar_label: contains
---

# contains
Given a tuple of keys, returns the keys which exist.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|tuple|Keys to check|Y|
|tkn|int|A session token|Only if sessions enabled|


:::note
With sessions enabled, this command only returns keys which exist in the session identified by the `tkn` parameter.
:::


## Returns

`tuple(bool, list)`
- `bool`: `True` if command successful, otherwise `False`
- `list` : the keys which exist


## Examples


```py
client = KvClient()
await client.open('ws://127.0.0.1:1987/')
(success, exist) = await client.contains(('k1','k2','k3'))
```
