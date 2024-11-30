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


## Returns
`int` : the number of keys


## Examples


```py
client = KvClient()
await client.open('ws://127.0.0.1:1987/')
count = await client.count()
```
