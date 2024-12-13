---
sidebar_position: 10
displayed_sidebar: clientApisSidebar
---

# Overview
Arrays are fixed size containers for JSON objects, strings or integers, implemented in contigious memory. Because they are fixed sized, their length must be defined at creation time, and cannot change.

- Rather than length, the terms "capacity" and "used" are preferred
    - `capacity`: the size of the array, allocated when `create()` is called
    - `used`: the number of items in the array. The array is full when `used == capacity`
    - `clear()` removes items, reducing `used`
- There are sorted and unsorted versions of string and integer arrays, but object arrays cannot be sorted
- Sorting is always in ascending order
- Sorted arrays allow for operations such as intersecting


## API

- Many API functions use indices, these begin at `0`
- In functions that use `start` and `stop` parameters, the `stop` is exclusive

:::note
`start` and `stop` parameters are similar to Python's `range()` function and splicing.

```py
values = await arrays.get_rng('some_array_name', start=0, stop=10)
```

Will return values from index `0` to and including `9`.
:::

<br/>

There is an API class per array type. You create an instance of the class, passing the `NdbClient` instance:

```py
from ndb.client import NdbClient
from ndb.arrays import IntArrays


client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# create API object
arrays = IntArrays(client)

# create array called 'example' with capacity of 4
await arrays.create('example', 4)

# in 'example' array, set four integers
# no `pos` is set so the next available position is used (which is 0 since the array is empty)
await arrays.set_rng('example', [100,50,200,10])

# get all values: stop is exclusive
allValues = await arrays.get_rng('example', start=0, stop=5)
print(allValues)

# you can omit 'stop' to return all

```

```bash title='Output: unsorted'
[100, 50, 200, 10]
```

The same data but with a sorted int array:

```py
from ndb.client import NdbClient
from ndb.arrays import SortedIntArrays


client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# this time use the SortedIntArrays API
sortedArrays = SortedIntArrays(client)

# create an array named 'sorted'
await sortedArrays.create('sorted', 4)

await sortedArrays.set_rng('sorted', [100,50,200,10])

# omit 'stop' to get all items
allValues = await sortedArrays.get_rng('sorted', start=0)
print(allValues)
```


```bash title='Output: sorted'
[10, 50, 100, 200]
```

## API Classes

|API Class|Storage Type|
|---|---|
|OArrays|JSON object|
|IntArrays|signed integer|
|StringArrays|string|
|SortedIntArrays|signed integer|
|SortedStringArrays|string|


## Sorted vs Unsorted

### Intersecting
- The intersecting algorithm requires the array is sorted
- The same will apply as other operations are added, such as union

### Swap Items
- Items can't be swapped in a sorted array as this would break ordering
