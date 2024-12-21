---
sidebar_position: 120
displayed_sidebar: clientApisSidebar
---

# remove_head

```py
async def remove_head(name: str) -> int
```

|Param|Description|
|---|---|
|name|Name of the list|

Remove the head node.


## Returns
The length of the list after removal.


## Raises
- `ResponseError`
    - `name` does not exist

