---
sidebar_position: 70
displayed_sidebar: clientApisSidebar
sidebar_label: sh_clear_set
---

# sh_clear_set
Deletes _all_ keys and sets new keys in a single call.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|dict|The new keys store after the existing keys have been deleted|Y|
|tkn|int|Session token|Y|


## Returns

`int` : the number of keys deleted


## Examples


```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()

# store user stats in a session
# periodically we reset counters to 0
await client.sh_clear_set({'stats_read':0, 'stats_received':0, 'stats_sent':0}, session.tkn)
```
