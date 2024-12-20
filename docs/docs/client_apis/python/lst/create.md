---
sidebar_position: 20
displayed_sidebar: clientApisSidebar
---

# create

```py
async def create(name: str) -> None
```

|Param|Description|
|---|---|
|name|Name of the list|


Creates a list with the given `name`.


## Raises
- `ResponseError`
    - `name` already exists


## Examples

```py
from ndb.client import NdbClient
from ndb.lists import ObjLists

client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# create API object
lists = ObjLists(client)

await lists.create('list1')
await lists.create('list2')

# add single item to list1
await lists.add('list1', {'prod_name':'TV', 'qty':1})
# add multiple items to list2
await lists.add('list2', [{'prod_name':'Lamp', 'qty':2}, {'prod_name':'Chair', 'qty':3}])

print(await lists.get_rng('list1', start=0))
print(await lists.get_rng('list2', start=0))
```

Output:

```py
[{'prod_name': 'TV', 'qty': 1}]
[{'prod_name': 'Lamp', 'qty': 2}, {'prod_name': 'Chair', 'qty': 3}]
```
