---
sidebar_position: 10
displayed_sidebar: clientApisSidebar
---

# Overview
Arrays are fixed size containers for JSON objects, strings or integers. They are implemented in contigious memory. Because they are fixed sized, their length must be defined at creation time, and cannot change.

There are sorted and unsorted versions of string and integer arrays, but object arrays are only unsorted.

Sorted arrays allow for operations such as intersecting two arrays.


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

# create array called 'example' of length 4
await arrays.create('example', 4)

# in 'example' array, starting at positon 0, set four integers
await arrays.set_rng('example', 0, [100,50,200,10])

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

# create a SortedIntArrays API 
arrays = SortedIntArrays(client)

# create an array named 'sorted'
await arrays.create('sorted', 4)

# note: no position is allowed on a sorted array
await arrays.set_rng('sorted', [100,50,200,10])

allValues = await arrays.get_rng('sorted', start=0, stop=5)
print(allValues)
```


```bash title='Output: sorted'
[10, 50, 100, 200]
```

## API Classes

|API Class Name|Storage Type|
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
- Items can't be swapped in a sorted array as this would break the sorting

### Clearing
Calling `clear()` on a sorted array does not overwrite the existing value because that would break the sorting, instead the cleared values are rotated to after the sorted values. See [`std::rotate`](https://en.cppreference.com/w/cpp/algorithm/rotate)

In the example above with `[10,50,100,200]`, if `clear()` is called on `50`, the array becomes:

```
[10, 100, 200, 50]
           ^   ^
       Last    Ignored
```

An internal variable marking `200` as the last value so subsequent calls to `get()`/`get_rng()` only consider `[10,100,200]`.

When `set()` is called again, `50` is overwritten and the array is sorted.