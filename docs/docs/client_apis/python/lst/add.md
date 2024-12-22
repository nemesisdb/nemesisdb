---
sidebar_position: 30
displayed_sidebar: clientApisSidebar
---

# add

```py
async def add(name: str, items: list[dict] | dict, pos = None) -> int:
```

|Param|Description|
|---|---|
|name|Name of the list|
|items|- Single item: dictionary<br/>- Multiple times: A list of dictionaries|
|pos|The position to insert the items. If out of bounds, the items are appened to the tail|


:::info
[`add_head()`](add_head) or [`add_tail()`](add_tail) can be used for convenience.
:::

The item(s) are inserted into the list at the given position.

With a list of four nodes:
```
A-B-D-E
```

Then call `add()` with `pos=2`, it will insert at `D`:
```
A-B-C-D-E
    ^
```

For multiple items, the insertion begins at `pos`.

With a list having nodes:

```
A-E-F-G
```

Then call `add()` with 3 items and `pos=1`:

```
A-B-C-D-E-F-G
  ^---^
```



## Raises
- `ResponseError`
    - `name` does not exist


## Examples

```py
from ndb.client import NdbClient
from ndb.lists import ObjLists

client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# create API object
lists = ObjLists(client)

await lists.create('my_list')
# add an item
await lists.add('my_list', {'username':'Bob'})
# add multiple items. pos isn't set, so add to tail by default
await lists.add('my_list', [{'username':'Brian'}, {'username':'Charles'}])
# add multiple items to the head. can use add() with pos=0, or add_head()
await lists.add_head('my_list', [{'username':'Alice'}, {'username':'Anna'}])


names = await lists.get_rng('my_list', start=0)
print(f'Before Brenda: {names}')

# insert Brenda at Brian
await lists.add('my_list', {'username':'Brenda'}, pos=3)

names = await lists.get_rng('my_list', start=0)
print(f'After: {names}')
```

Output:

```py
Before Brenda: [{'username': 'Alice'}, {'username': 'Anna'}, {'username': 'Bob'}, {'username': 'Brian'}, {'username': 'Charles'}]
After: [{'username': 'Alice'}, {'username': 'Anna'}, {'username': 'Bob'}, {'username': 'Brenda'}, {'username': 'Brian'}, {'username': 'Charles'}]
```
