---
sidebar_position: 40
displayed_sidebar: clientApisSidebar
---

# set_rng

```py title='Object Array'
async def set_rng(name: name: str, items: List[dict], pos = None) -> None:
```

```py title='Int Array'
async def set_rng(name: str, items: List[int], pos = None) -> None:
```

```py title='String Array'
async def set_rng(name: str, items: List[str], pos = None) -> None:
```

```py title='Sorted Int Array'
async def set_rng(name: str, items: List[int]) -> None:
```

```py title='Sorted String Array'
async def set_rng(name: str, items: List[str]) -> None:
```


|Param|Description|
|---|---|
|name|Name of the array|
|items|A list of items to store|
|pos|__Unsorted arrays only__ <br/> The position to begin storing. The existing value is overwritten.<br/>If `None` the next available position is used.|



## Array Type Differences
- Sorted arrays do not accept a `pos` parameter because the position is determined by the sorted order


## Raises
- `ResponseError` if query fails
    - `name` does not exist
    - `item` is incorrect type
- `ValueError` caught before query is sent
    - `name` is empty


## Examples

Same as example in [set()](./set#examples) but with `set_rng()`.

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
records = [{'name':'Alice', 'modules':['Geography', 'Biology']},
           {'name':'Bob', 'modules':['Art', 'Philosophy']},
           {'name':'Charles', 'modules':['Music']}]

await objectArrays.set_rng('student_records', records)

# sorted names
await sortedStrArrays.set_rng('student_names_sorted', ['Charles', 'Bob', 'Alice'])
# sorted leaderboard scores
await sortedIntArrays.set_rng('student_scores', [21, 20, 18])

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
