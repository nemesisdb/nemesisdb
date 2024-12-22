---
sidebar_position: 170
displayed_sidebar: clientApisSidebar
---

# splice

```py 
async def splice(self, destName: str, srcName: str, srcStart: int, srcEnd = None, destPos = None) -> None
```

|Param|Description|
|---|---|
|destName|The list to receive the spliced nodes. This is created if it doesn't exist.|
|srcName|The list to splice|
|srcStart|The position in `srcName` to begin splicing|
|srcEnd|The exclusive position in `srcName` to stop splicing. If `None`, to end is used|
|destPos|The position in `destName` where the spliced nodes will be inserted. If `None`, nodes be appended to the tail|


Move source nodes in range `[srcStart, srcEnd)` and to the destination list, inserting at `destPos`.

If the destination list does not exist, it is created.

No nodes are copied, only pointers are amended.


## Raises
- `ResponseError` if query fails
  - `srcName` does not exist
- `ValueError`
  - `destName` is empty
  - `srcName` is empty
- `TypeError`
  - `srcEnd` not an int
  - `destPos` not an int


## Examples

```py
from ndb.client import NdbClient
from ndb.lists import ObjLists

client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

lists = ObjLists(client)

await lists.create('src')

data = []
for i in range(0,10):
  data.append({f'k{i}':i})

await lists.add('src', data)

src_list = await lists.get_rng('src', start=0)
print(f'Source\n{src_list}')

# move k3, k4 to a new list
print('Splicing 1')
await lists.splice('dest', 'src', srcStart=3, srcEnd=5)

src_list = await lists.get_rng('src', start=0)
dest_list = await lists.get_rng('dest', start=0)

print(f'\tSource: {src_list}')
print(f'\tDest: {dest_list}')

# move k5 to k9, appending to destination
print('Splicing 2')
await lists.splice('dest', 'src', srcStart=3)

src_list = await lists.get_rng('src', start=0)
dest_list = await lists.get_rng('dest', start=0)

print(f'\tSource: {src_list}')
print(f'\tDest: {dest_list}')
```

```
Source
[{'k0': 0}, {'k1': 1}, {'k2': 2}, {'k3': 3}, {'k4': 4}, {'k5': 5}, {'k6': 6}, {'k7': 7}, {'k8': 8}, {'k9': 9}]
Splicing 1
  Source: [{'k0': 0}, {'k1': 1}, {'k2': 2}, {'k5': 5}, {'k6': 6}, {'k7': 7}, {'k8': 8}, {'k9': 9}]
  Dest: [{'k3': 3}, {'k4': 4}]
Splicing 2
  Source: [{'k0': 0}, {'k1': 1}, {'k2': 2}]
  Dest: [{'k3': 3}, {'k4': 4}, {'k5': 5}, {'k6': 6}, {'k7': 7}, {'k8': 8}, {'k9': 9}]
```