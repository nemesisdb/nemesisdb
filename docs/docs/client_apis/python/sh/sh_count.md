---
sidebar_position: 80
displayed_sidebar: clientApisSidebar
sidebar_label: sh_count
---

# sh_count
Returns the number of keys in a session. 


```py
sh_count(tkn: int) -> int:
```


## Returns
The number of keys


## Raises
- `ResponseError` if query fails


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()

await client.sh_set(session.tkn, {'stats_read':0, 'stats_received':0, 'stats_sent':0})
count = client.sh_count(session.tkn)
print(count) # 3
```