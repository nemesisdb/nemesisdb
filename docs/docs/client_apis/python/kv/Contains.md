---
sidebar_position: 9
displayed_sidebar: clientApisSidebar
sidebar_label: kv_contains
---

# kv_contains
Given a tuple of keys, returns the keys which exist.

```py
kv_contains(keys: tuple) -> List[str]
```

|Param|Description|
|--|--|
|keys|Keys to check|


## Returns
The keys that exist


## Raises
- `ResponseError` if query fails



## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

await client.kv_set({'k1':10, 'k2':20})

exist = await client.kv_contains(('k1','k2','k3'))

print(exist)
# ['k1', 'k2']
```
