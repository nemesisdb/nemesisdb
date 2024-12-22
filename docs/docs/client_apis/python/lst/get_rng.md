---
sidebar_position: 100
displayed_sidebar: clientApisSidebar
---

# get_rng

```py
async def get_rng(self, name: str, start: int, stop=None) -> List[dict]
```

|Param|Description|
|---|---|
|name|Name of the list|
|start|Position of the first item to get|
|stop|Exclusive position of the final item. `stop` being `None` means get to end of list|


:::note
The range retrieved is `[start, stop)`.
:::


## Raises
- `ResponseError`
    - `name` does not exist
- `ValueError`
    - `start < 0`
    - `stop < 0`
    - `stop < start`
- `TypeError`
  - `stop` not an int


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

# get everything but only print first 3
everything = await lists.get_rng('data', start=0)
print(everything[0:3])

# or just get the first three
firstThree = await lists.get_rng('data', start=0, stop=3)
print(firstThree)

print(await lists.get_rng('data', start=3, stop=6))
```

```
[{'k0': 0}, {'k1': 1}, {'k2': 2}]
[{'k0': 0}, {'k1': 1}, {'k2': 2}]
[{'k3': 3}, {'k4': 4}, {'k5': 5}]
```