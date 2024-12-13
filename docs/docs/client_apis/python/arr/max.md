---
sidebar_position: 320
displayed_sidebar: clientApisSidebar
sidebar_label: max (Sorted Only)
---

# max

```py 
async def max(name: str, n = 1) -> List[str] | List[int]
```

|Param|Description|
|---|---|
|name|Name of the array|
|n|How many of the highest values to return|

Retrieves the minimum value(s). `n` can be used to get lowest `n` values.


## Array Type Differences
- Only applies to sorted arrays


## Raises
- `ResponseError`
    - `name` does not exist
    - `n` is out of bounds
- `ValueError` caught before query is sent
    - `name` is empty
    - `n < 1`


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

sortedInts = SortedIntArrays(client)
await sortedInts.create('scores', 6)
await sortedInts.set_rng('scores', [50,35,75,100,15,82])

bottom3 = await sortedInts.min('scores', n=3)
top3 = await sortedInts.max('scores', n=3)

print(f'Best 3: {top3}')
print(f'Worst 3: {bottom3}')
```

Output
```
Best 3: [100, 82, 75]
Worst 3: [15, 35, 50]
```

<br/>
