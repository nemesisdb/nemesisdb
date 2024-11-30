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


## Returns
`list` : the keys which exist


## Examples


```py
client = KvClient()
await client.open('ws://127.0.0.1:1987/')
exist = await client.contains(('k1','k2','k3'))
```
