---
sidebar_position: 70
displayed_sidebar: clientApisSidebar
sidebar_label: sh_clear_set
---

# sh_clear_set
Deletes _all_ keys and sets new keys in a single call.



```py
sh_clear_set(tkn: int, keys: dict) -> int:
```

|Param|Description|
|--|--|
|keys|dict|The new keys store after the existing keys have been deleted|
|tkn|int|Session token|



## Returns
The number of keys deleted


## Raises
- `ResponseError` if query fails


## Examples
```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create()

# store user stats in a session
# periodically reset counters to 0
await client.sh_clear_set(session.tkn, {'stats_read':0, 'stats_received':0, 'stats_sent':0})
```
