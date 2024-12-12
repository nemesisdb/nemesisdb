---
sidebar_position: 70
displayed_sidebar: clientApisSidebar
---

# clear

```py 
async def clear(name: str, start: int, stop: int) -> None
```

|Param|Description|
|---|---|
|name|Name of the array|
|start|Postion to start clearing|
|stop|Position to stop clearing, exclusive|

This clears items in range `[start, stop)`.

After calling `clear()`, `used()` decreases by the number of items cleared (`stop-start`).


## Array Type Differences
None


## Raises
- `ResponseError` if query fails
    - `name` does not exist
    - `start` out of bounds
- `ValueError` caught before query is sent
    - `name` is empty


## Examples

Fill array, clear two items and then set.
```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

arrays = IntArrays(client)
await arrays.create('values', 4)

# fill array
await arrays.set_rng('values', [56,34,78,45])

values = await arrays.get_rng('values', start=0)
print(values)

# array is full
try:
    await arrays.set_rng('values', [99, 66])
except ResponseError:
    print("Array full")

# clear 34 and 78 then try again
await arrays.clear('values', start=1, stop=3)
await arrays.set_rng('values', [99,66])

values = await arrays.get_rng('values', start=0)
print(values)
```


```bash title='Output'
[56, 34, 78, 45]
Array full
[56, 45, 99, 66]
```
<br/>

Fill array, clear all items and then fill again.
```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

arrays = IntArrays(client)
await arrays.create('values', 4)

# fill array
await arrays.set_rng('values', [56,34,78,45])

values = await arrays.get_rng('values', start=0)
print(values)

# clear all and set new
await arrays.clear('values', start=0, stop=5)
await arrays.set_rng('values', [33,11,99,66])

values = await arrays.get_rng('values', start=0)
print(values)
```

```bash title='Output'
[56, 34, 78, 45]
[33, 11, 99, 66]
```