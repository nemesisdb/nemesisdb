---
sidebar_position: 300
displayed_sidebar: clientApisSidebar
sidebar_label: intersect (Sorted Only)
---

# intersect

```py 
async def intersect(arrA: str, arrB: str) -> List[str] | List[int]
```

|Param|Description|
|---|---|
|name|Name of the array|
|arrA|First array to intersect|
|arrB|Second array to intersect|

Intersects `arrA` with `arrB`.


## Array Type Differences
- Only applies to sorted arrays


## Raises
- `ResponseError` if query fails
    - `arrA` does not exist
    - `arrB` does not exist
- `ValueError` caught before query is sent
    - `arrA` is empty
    - `arrB` is empty
    - `arrA == arrB` 


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

sortedInts = SortedIntArrays(client)
await sortedInts.create('array1', 5)
await sortedInts.create('array2', 6)

await sortedInts.set_rng('array1', [25,10,50,100,80])
await sortedInts.set_rng('array2', [10,25,100,120,200,5])

intersected = await sortedInts.intersect('array1', 'array2')
print(intersected)
```

Output
```
[10, 25, 100]
```

<br/>

```py
async def createData(s: int, size: int):
    from random import sample, seed
    seed(s)
    return sample(range(0, size*4), size)


client = NdbClient()
connect = client.open('ws://127.0.0.1:1987/')

print('Creating data') 
data1 = await createData(7,500)
data2 = await createData(9,500)

await connect

sortedInts = SortedIntArrays(client)

print('Creating arrays') 
await sortedInts.create('array1', 500)
await sortedInts.create('array2', 500)

print('Storing data')
await sortedInts.set_rng('array1', data1)
await sortedInts.set_rng('array2', data2)

print('Intersecting')
intersected = await sortedInts.intersect('array1', 'array2')
print(f'Intersected has: {len(intersected)} values')
```