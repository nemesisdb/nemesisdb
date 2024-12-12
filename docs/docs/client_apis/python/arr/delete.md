---
sidebar_position: 80
displayed_sidebar: clientApisSidebar
---

# delete

```py 
async def delete(name: str) -> None
```

|Param|Description|
|---|---|
|name|Name of the array|

Delete the given array, releasing the memory.


## Array Type Differences
None


## Raises
- `ResponseError` if query fails
    - `name` does not exist
- `ValueError` caught before query is sent
    - `name` is empty


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

arrays = StringArrays(client)
await arrays.create('names', 4)
await arrays.set_rng('names', ['hello', 'world', 'goodbye', 'world'])

await arrays.delete('names')

# we can create an array with the same name after deleting the original
await arrays.create('names', 3)
await arrays.set_rng('names', ['hello', 'world', 'again'])
```