---
sidebar_position: 30
displayed_sidebar: clientApisSidebar
---

# set

```py title='Object Array'
async def set(name: str, item: dict, pos = None) -> None:
```

```py title='Int Array'
async def set(name: str, item: int, pos = None) -> None:
```

```py title='String Array'
async def set(name: str, item: str, pos = None) -> None:
```

```py title='Sorted Int Array'
async def set(name: str, item: int) -> None:
```

```py title='Sorted String Array'
async def set(name: str, item: str) -> None:
```


|Param|Description|
|---|---|
|name|Name of the array|
|item|The item to store|
|pos|__Unsorted arrays only__ <br/> The position to store. The existing value is overwritten.<br/>If `None` the next available position is used.|



## Array Type Differences
- Sorted arrays do not accept a `pos` parameter because the position is determined by the sorted order


## Raises
- `ResponseError` if query fails
    - `name` does not exist
    - `item` is incorrect type
- `ValueError` caught before query is sent
    - `name` is empty


## Examples

:::note
In the example, `set()` is called multiple times for illustration, a better approach is to use `set_rng()`, i.e.:

```py
await sortedStrArrays.set_rng('student_names_sorted', ['Charles', 'Bob', 'Alice'])
```
:::

```py
from pprint import pprint
from ndb.client import NdbClient
from ndb.arrays import SortedIntArrays, ObjArrays, SortedStrArrays

client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

objectArrays = ObjArrays(client)
sortedStrArrays = SortedStrArrays(client)
sortedIntArrays = SortedIntArrays(client)

await objectArrays.create('student_records', 3)
await sortedStrArrays.create('student_names_sorted', 3)
await sortedIntArrays.create('student_scores', 3)

# student records as json objects
await objectArrays.set('student_records', {'name':'Alice', 'modules':['Geography', 'Biology']})
await objectArrays.set('student_records', {'name':'Bob', 'modules':['Art', 'Philosophy']})
await objectArrays.set('student_records', {'name':'Charles', 'modules':['Music']})

# sorted names
await sortedStrArrays.set('student_names_sorted', 'Charles')
await sortedStrArrays.set('student_names_sorted', 'Bob')
await sortedStrArrays.set('student_names_sorted', 'Alice')

# sorted leaderboard scores
await sortedIntArrays.set('student_scores', 21)
await sortedIntArrays.set('student_scores', 20)
await sortedIntArrays.set('student_scores', 18)

records = await objectArrays.get_rng('student_records', start=0)
names = await sortedStrArrays.get_rng('student_names_sorted', start=0)
scores = await sortedIntArrays.get_rng('student_scores', start=0)

print('Records')
pprint(records)
print('Names')
print(names)
print('Scores')
print(scores)
```

Output:
```py
Records
[{'modules': ['Geography', 'Biology'], 'name': 'Alice'},
 {'modules': ['Art', 'Philosophy'], 'name': 'Bob'},
 {'modules': ['Music'], 'name': 'Charles'}]
Names
['Alice', 'Bob', 'Charles']
Scores
[18, 20, 21]
```
