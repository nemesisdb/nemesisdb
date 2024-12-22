---
sidebar_position: 70
displayed_sidebar: clientApisSidebar
---

# get

```py
async def get(name: str, pos: int) -> dict
```

|Param|Description|
|---|---|
|name|Name of the list|
|pos|Position of the item|

Get an item from a list.


## Raises
- `ResponseError`
    - `name` does not exist
- `ValueError`
    - `start < 0`


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# create API object
lists = ObjLists(client)

await lists.create('books')

await lists.add('books', [{'title':'Harry Potter'}, {'title':'Moby Dick'}, {'title':'War and Peace'}])
print(await lists.get('books', pos=1))
```

```
{'title': 'Moby Dick'}
```