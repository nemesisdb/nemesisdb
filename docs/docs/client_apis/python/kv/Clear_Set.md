---
sidebar_position: 7
displayed_sidebar: clientApisSidebar
sidebar_label: kv_clear_set
---

# kv_clear_set
Deletes _all_ keys and sets new keys in a single call.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|dict|The new keys store after the existing keys have been deleted|Y|


## Returns

`int` : the number of keys deleted


## Examples


```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')
# using the db to store stats, after a period of time we want to reset counters
# we could use set() on these three keys, but if the db only stores these
# stat keys, we can clear_set()
nKeysDeleted = await client.kv_clear_set({'stats_total':0, 'stats_visitors':0, 'stats_blocked':0})
```
