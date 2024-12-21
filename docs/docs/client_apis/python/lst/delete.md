---
sidebar_position: 140
displayed_sidebar: clientApisSidebar
---

# delete

```py 
async def delete(name: str) -> None
```

|Param|Description|
|---|---|
|name|Name of the list|

Delete the given list.



## Raises
- `ResponseError` if query fails
    - `name` does not exist
- `ValueError`
    - `name` is empty

