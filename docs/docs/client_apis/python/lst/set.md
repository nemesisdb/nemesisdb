---
sidebar_position: 60
displayed_sidebar: clientApisSidebar
---

# set

```py
async def set(self, name: str, items=list[dict]|dict, start=0) -> None:
```

|Param|Description|
|---|---|
|name|Name of the list|
|items|- Single item: dictionary<br/>- Multiple times: A list of dictionaries|
|start|Position to start setting|

Overwrite existing node(s). 


## Raises
- `ResponseError`
    - `name` does not exist
- `ValueError`
    - `start < 0`
- `TypeError`
    - `items` is not `dict` or `list`


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# create API object
lists = ObjLists(client)

await lists.create('books')

await lists.add('books', [{'title':'Harry Potter'}, {'title':'Moby Dick'}, {'title':'War and Peace'}])
print(await lists.get_rng('books', start=0))

# overwrite Moby Dick 
await lists.set('books', {'title':'Dracula'}, start=1)
print(await lists.get_rng('books', start=0))
```

```
[{'title': 'Harry Potter'}, {'title': 'Moby Dick'}, {'title': 'War and Peace'}]
[{'title': 'Harry Potter'}, {'title': 'Dracula'}, {'title': 'War and Peace'}]
```