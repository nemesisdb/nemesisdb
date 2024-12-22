---
sidebar_position: 10
displayed_sidebar: clientApisSidebar
---

# Overview
A list is a node based container, implemented as a doubly linked list. 

There is only one type of list, storing JSON objects and is unsorted.


## API

- Many API functions use indices, these begin at `0`
- In functions that use `start` and `stop` parameters, the `stop` is exclusive

:::note
`start` and `stop` parameters are similar to Python's `range()` function and splicing.

```py
values = await lists.get_rng('my_list', start=0, stop=10)
```

Will return values from index `0` to and including `9`.
:::

<br/>

After creating and connecting the `NdbClient`, create an instance of `ObjLists`:


```py title='List as a stack'
from ndb.client import NdbClient
from ndb.lists import ObjLists

client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# create API object
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

## API Classes

|API Class|Storage Type|
|---|---|
|ObjLists|JSON object|
