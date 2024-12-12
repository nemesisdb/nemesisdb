---
sidebar_position: 20
displayed_sidebar: clientApisSidebar
---

# create

```py
async def create(name: str, length: int) -> None
```

|Param|Description|
|---|---|
|name|Name of the array|
|length|Length of the array|

:::note
The length is fixed, the array cannot extend
:::

<br/>

The `name` must only be unique amongst arrays of the same type, i.e. you can create an object array called `students` and an int array also called `students`.

## Array Type Differences
None


## Raises
- `ResponseError` if query fails
    - if `name` already exists    
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

# can use the same name because arrays are different type
await unsortedArrays.create('my_array', 4)
await sortedArrays.create('my_array', 4)

await unsortedArrays.set_rng('my_array', 0, [100,50,200,10])
await sortedArrays.set_rng('my_array', [100,50,200,10])

# omit 'stop', get all values
unsortedValues = await unsortedArrays.get_rng('my_array', start=0)
sortedValues = await sortedArrays.get_rng('my_array', start=0)

print(unsortedValues)
print(sortedValues)
```

Output:

```py
[100, 50, 200, 10]
[10, 50, 100, 200]
```
