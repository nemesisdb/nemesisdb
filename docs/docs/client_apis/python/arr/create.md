---
sidebar_position: 20
displayed_sidebar: clientApisSidebar
---

# create

```py
async def create(name: str, capacity: int) -> None
```

|Param|Description|
|---|---|
|name|Name of the array.<br/>The `name` must only be unique amongst arrays of the same type, i.e. you can create an object array called `students` and an integer array also called `students`|
|capacity|Maximum length of the array|

:::note
The capacity is fixed, the array's length/capacity cannot increase.
:::

If `capacity` exceeds the `arrays:maxCapacity` in the server config a ResponseError is raised, containing the `Bounds` status. 

The `capacity` is the maximum length of the array. As new values are set, the `used()` increases. 
When `used() == capacity()` the array is full. Calls to `clear()` reduces `used()`. 

In an unsorted array you can use `set()` to overwrite existing values, so the `used()` does not change.



## Array Type Differences
None


## Raises
- `ResponseError`
    - `name` already exists
    - `capacity` exceeds maximum set in the server config
- `ValueError` caught before query is sent
    - `name` is empty
    - `len` is `<= 0`


## Examples

```py
from ndb.client import NdbClient
from ndb.arrays import IntArrays, SortedIntArrays

client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# create API object for unsorted and sorted int array
unsortedArrays = IntArrays(client)
sortedArrays = SortedIntArrays(client)

# create two arrays, each with a capacity of 4
await unsortedArrays.create('my_array1', 4)
await sortedArrays.create('my_array2', 4)

# omit 'pos' to being set at next available position
await unsortedArrays.set_rng('my_array1', [100,50,200,10])
await sortedArrays.set_rng('my_array2', [100,50,200,10])

# omit 'stop', get values to end
unsortedValues = await unsortedArrays.get_rng('my_array1', start=0)
sortedValues = await sortedArrays.get_rng('my_array2', start=0)

print(unsortedValues)
print(sortedValues)
```

Output:

```py
[100, 50, 200, 10]
[10, 50, 100, 200]
```
