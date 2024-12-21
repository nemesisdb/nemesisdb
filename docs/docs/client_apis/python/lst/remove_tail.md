---
sidebar_position: 130
displayed_sidebar: clientApisSidebar
---

# remove_tail

```py
async def remove_tail(name: str) -> int
```

|Param|Description|
|---|---|
|name|Name of the list|

Remove the tail node.


## Returns
The length of the list after removal.


## Raises
- `ResponseError`
    - `name` does not exist
