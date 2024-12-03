---
sidebar_position: 7
displayed_sidebar: clientApisSidebar
sidebar_label: kv_clear_set
---

# kv_clear_set
Deletes _all_ keys then sets new keys in a single call.


```py
kv_clear_set(keys: dict)
```

|Param|Description|
|--|--|
|keys|The new keys stored after the existing keys have been deleted|


## Returns
`None`


## Raises
- `ResponseError` if query fails


## Examples

```py
# using the db to store stats, after a period of time we reset counters
nKeysDeleted = await client.kv_clear_set({'stats_total':0, 'stats_visitors':0, 'stats_blocked':0})
```
