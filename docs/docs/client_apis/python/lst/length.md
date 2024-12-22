---
sidebar_position: 160
displayed_sidebar: clientApisSidebar
---

# length

```py 
async def length(name: str) -> None
```

|Param|Description|
|---|---|
|name|Name of the list|


## Returns
The number of nodes in the list.


## Raises
- `ResponseError` if query fails
  - `name` does not exist
- `ValueError`
  - `name` is empty