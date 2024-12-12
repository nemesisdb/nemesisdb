---
sidebar_position: 60
displayed_sidebar: clientApisSidebar
---

# get_rng

```py 
async def get_rng(name: str, start: int, stop = None) -> List[dict] | List[str] | List[int]
```

`stop` is exclusive based.

|Param|Description|
|---|---|
|name|Name of the array|
|start|Position of the first item to retrieve|
|stop|The last index, exclusive|


## Returns
The items requested.


## Array Type Differences
None


## Raises
- `ResponseError` if query fails
    - `name` does not exist
    - `start` out of bounds
- `ValueError` caught before query is sent
    - `name` is empty
    - `start > stop`


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

sortedInts = SortedIntArrays(client)
await sortedInts.create('scores', 5)
await sortedInts.set_rng('scores', [50,102,95,64,22])

scores = await sortedInts.get_rng('scores', start=0)

print(f'Ascending: {scores}')
print(f'Descending: {scores[::-1]}')
print(f'High: {scores[-1]}, '
      f'Low: {scores[0]}, '
      f'Top Three: {scores[-1:-4:-1]}')


# get just the top three
scores = await sortedInts.get_rng('scores', start=2)
print(f'Top Three with get_rng(): {scores[::-1]}')
```

```
Ascending: [22, 50, 64, 95, 102]
Descending: [102, 95, 64, 50, 22]
High: 102, Low: 22, Top Three: [102, 95, 64]
Top Three with get_rng(): [102, 95, 64]
```