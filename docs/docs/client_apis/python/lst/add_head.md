---
sidebar_position: 40
displayed_sidebar: clientApisSidebar
---

# add_head

```py
async def add_head(name: str, items=list[dict]|dict) -> None:
```

|Param|Description|
|---|---|
|name|Name of the list|
|items|- Single item: dictionary<br/>- Multiple times: A list of dictionaries|

The item(s) are inserted into at the head:

- adding a single item, the item becomes the head
- adding multiple items, the first item becomes the head


`add_head()` and `remove_head()` can be called to use a list as a stack.


## Raises
- `ResponseError`
    - `name` does not exist


## Examples

```py title='List as a stack'
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

lists = ObjLists(client)

await lists.create('stack')

print('Push')
await lists.add_head('stack', {'name':'first'})
await lists.add_head('stack', {'name':'second'})
await lists.add_head('stack', {'name':'third'})

for item in await lists.get_rng('stack', start=0):
  print(item)

print('Pop')
await lists.remove_head('stack')

for item in await lists.get_rng('stack', start=0):
  print(item)
```

```
Push
{'name': 'third'}
{'name': 'second'}
{'name': 'first'}
Pop
{'name': 'second'}
{'name': 'first'}
```