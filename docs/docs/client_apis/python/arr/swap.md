---
sidebar_position: 200
displayed_sidebar: clientApisSidebar
sidebar_label: swap (Unsorted Only)
---

# swap

```py 
async def swap(name: str, posA: int, posB: int) -> None
```

|Param|Description|
|---|---|
|name|Name of the array|
|posA|Swap item at `posA` with the item at `posB`|
|posB|Swap item at `posA` with the item at `posB`|



Swap the items at positions `posA` and `posB`.


## Array Type Differences
- Only applies to unsorted arrays


## Raises
- `ResponseError` if query fails
    - `name` does not exist
- `ValueError` caught before query is sent
    - `name` is empty
    - `posA == posB` 


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

arrays = StringArrays(client)
await arrays.create('values', capacity=4)

await arrays.set_rng('values', ['a', 'b', 'c', 'd'])
print(await arrays.get_rng('values', start=0))

await arrays.swap('values', 0, 3)
print(await arrays.get_rng('values', start=0))
```

Output
```
['a', 'b', 'c', 'd']
['d', 'b', 'c', 'a']
```
