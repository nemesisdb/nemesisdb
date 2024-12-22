---
sidebar_position: 7
displayed_sidebar: clientApisSidebar
sidebar_label: clear_set
---

# clear_set

```py
async def clear_set(keys: dict) -> int
```

|Param|Description|
|--|--|
|keys|The new keys stored after the existing keys are deleted|

Deletes _all_ keys then sets new keys in a single call.


## Returns
The number of keys deleted.


## Raises
- `ResponseError`


## Examples

```py
nKeysDeleted = await kv.clear_set({'stats_total':0, 'stats_visitors':0, 'stats_blocked':0})
```
