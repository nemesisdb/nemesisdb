---
sidebar_position: 9
displayed_sidebar: clientApisSidebar
sidebar_label: contains
---

# contains


```py
async def contains(keys: tuple) -> List[str]
```

|Param|Description|
|--|--|
|keys|Keys to check|

Given a tuple of keys, returns the keys which exist.


## Returns
The keys that exist


## Raises
- `ResponseError` if query fails


## Examples

```py
await kv.set({'k1':10, 'k2':20})
exist = await kv.contains(('k1','k2','k3'))

print(exist) # ['k1', 'k2']
```
