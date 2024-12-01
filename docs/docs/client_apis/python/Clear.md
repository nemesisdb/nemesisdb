---
sidebar_position: 6
displayed_sidebar: clientApisSidebar
sidebar_label: clear
---

# kv_clear
Deletes _all_ keys.


- To delete select key(s) use [rmv](./Rmv)
- To delete all keys and set new keys in a single call, use [clear_set](./Clear_Set)


## Returns
`int` : the number of keys cleared


## Examples


```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')
# store some keys
count = await client.kv_clear()
print(f'Cleared {count} keys')
```