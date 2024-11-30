---
sidebar_position: 10
displayed_sidebar: clientApisSidebar
sidebar_label: keys
---

# keys
Returns all keys, excluding values.


## Returns
`list` : the keys


## Examples


```py
client = KvClient()
await client.open('ws://127.0.0.1:1987/')
allKeys = await client.keys()
```
