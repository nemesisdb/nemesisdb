---
sidebar_position: 50
displayed_sidebar: clientApisSidebar
---

# add_tail

```py
async def add_tail(self, name: str, items=list[dict]|dict) -> int:
```

|Param|Description|
|---|---|
|name|Name of the list|
|items|- Single item: dictionary<br/>- Multiple times: A list of dictionaries|

The item(s) are appended to the tail:

- adding a single item, the item becomes the tail
- adding multiple items, the last item becomes the tail


`add_head()` and `remove_head()` can be called to use a list as a stack.


## Returns
The position of the insert (i.e. the position of the tail before the insertion).


## Raises
- `ResponseError`
    - `name` does not exist
