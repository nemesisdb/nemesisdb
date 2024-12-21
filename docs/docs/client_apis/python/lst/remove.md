---
sidebar_position: 110
displayed_sidebar: clientApisSidebar
---

# remove

```py
async def remove(name: str, start: int, stop = None) -> int
```

|Param|Description|
|---|---|
|name|Name of the list|
|start|Position to begin removal. If start is out of bounds, it is not an error|
|stop|Exclusive position to stop removal|


Remove item(s) from a list. 

There are convenience functions `remove_head()` and `remove_tail()`.

:::note
The range retrieved is `[start, stop)`.
:::


## Returns
The length of the list after removal.


## Raises
- `ResponseError`
    - `name` does not exist
- `ValueError`
    - `start < 0`
    - `stop < 0`
    - `stop < start`


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# create API object
lists = ObjLists(client)

await lists.create('data')

data = []
for i in range(0,10):
data.append({f'k{i}':i})

await lists.add('data', data)
print(await lists.get_rng('data', start=0))

# remove: k3, k4 and k5
await lists.remove('data', start=3, stop=6)
print(await lists.get_rng('data', start=0))

# remove k8 and k9 (stop is None, so remove to end)
await lists.remove('data', start=5)
print(await lists.get_rng('data', start=0))
```

```
[{'k0': 0}, {'k1': 1}, {'k2': 2}, {'k3': 3}, {'k4': 4}, {'k5': 5}, {'k6': 6}, {'k7': 7}, {'k8': 8}, {'k9': 9}]
[{'k0': 0}, {'k1': 1}, {'k2': 2}, {'k6': 6}, {'k7': 7}, {'k8': 8}, {'k9': 9}]
[{'k0': 0}, {'k1': 1}, {'k2': 2}, {'k6': 6}, {'k7': 7}]
```