---
sidebar_position: 110
displayed_sidebar: clientApisSidebar
---

# capacity

```py 
async def capacity(name: str) -> int
```

|Param|Description|
|---|---|
|name|Name of the array|

Returns the capacity of the array. The capacity is the same as defined with `create()` and does not change.

This is in contrast to [`used()`](./used) which changes as items are set and cleared.

When the value of `used() == capacity()` the array is full.


## Array Type Differences
None


## Raises
- `ResponseError` if query fails
    - `name` does not exist
- `ValueError` caught before query is sent
    - `name` is empty


