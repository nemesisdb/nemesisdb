---
sidebar_position: 120
displayed_sidebar: clientApisSidebar
---

# used

```py 
async def used(name: str) -> int
```

|Param|Description|
|---|---|
|name|Name of the array|

Returns the number of items in the array. This can differ from the `capacity()` because capacity is how many items the array *can* store, whilst `used()` is how many items are *currently* stored.

When `used() == capacity()` the array is full.


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
await arrays.create('values', capacity=4)

# no items stored
capacity = await arrays.capacity('values')
used = await arrays.used('values')
print (f'1. capacity: {capacity}, used: {used}')

# store 3 items, leaving 1 array slot free
await arrays.set_rng('values', ['a', 'b', 'c'])
capacity = await arrays.capacity('values')
used = await arrays.used('values')
print (f'2. capacity: {capacity}, used: {used}')

# clear the first 2 items
await arrays.clear('values', start=0, stop=2)

capacity = await arrays.capacity('values')
used = await arrays.used('values')
print (f'3. capacity: {capacity}, used: {used}')
```

Output
```
1. capacity: 4, used: 0
2. capacity: 4, used: 3
3. capacity: 4, used: 1
```

1. Used is 0 because no items are stored
2. Three items are stored, so `used` is 3
3. Two items were cleared, so now `used` is 1

Capacity never changes after `create()`.