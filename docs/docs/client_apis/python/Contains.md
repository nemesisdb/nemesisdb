---
sidebar_position: 9
displayed_sidebar: clientApisSidebar
sidebar_label: contains
---

# kv_contains
Given a tuple of keys, returns the keys which exist.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|tuple|Keys to check|Y|


## Returns
`list` : the keys which exist


## Examples


```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

await client.kv_set({'k1':10, 'k2':20})

exist = await client.kv_contains(('k1','k2','k3'))

print(exist)
# ['k1', 'k2']
```
