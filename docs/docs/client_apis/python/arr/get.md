---
sidebar_position: 50
displayed_sidebar: clientApisSidebar
---

# get

```py 
async def get(name: str, pos: int) -> dict | str | int
```

|Param|Description|
|---|---|
|name|Name of the array|
|pos|The position of the item|


## Returns
The item requested.


## Array Type Differences
None


## Raises
- `ResponseError` if query fails
    - `name` does not exist
    - `pos` out of bounds
- `ValueError` caught before query is sent
    - `name` is empty


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

values = [56,34,78,45]

# unsorted
unsortedInts = IntArrays(client)
await unsortedInts.create('values', 4)
await unsortedInts.set_rng('values', values)

# sorted
sortedInts = SortedIntArrays(client)
await sortedInts.create('values', 4)
await sortedInts.set_rng('values', values)

first = await unsortedInts.get('values', 0)
last = await unsortedInts.get('values', 3)
print(f'Unsorted: {first} and {last}')

first = await sortedInts.get('values', 0)
last = await sortedInts.get('values', 3)
print(f'Sorted: {first} and {last}')
```

```
Unsorted: 56 and 45
Sorted: 34 and 78
```