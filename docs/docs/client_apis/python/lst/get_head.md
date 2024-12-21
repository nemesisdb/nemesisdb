---
sidebar_position: 80
displayed_sidebar: clientApisSidebar
---

# get_head

```py
async def get_head(name: str) -> dict
```

|Param|Description|
|---|---|
|name|Name of the list|

Get the head item from a list.


## Raises
- `ResponseError`
    - `name` does not exist


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

lists = ObjLists(client)

await lists.create('books')

await lists.add('books', [{'title':'Harry Potter'}, {'title':'Moby Dick'}, {'title':'War and Peace'}])
print(await lists.get_head('books'))
```

```
{'title': 'Harry Potter'}
```