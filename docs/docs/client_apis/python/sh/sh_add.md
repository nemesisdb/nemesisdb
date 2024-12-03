---
sidebar_position: 40
displayed_sidebar: clientApisSidebar
---

# sh_add
Stores keys but does not overwrite if it already exists.

To overwrite an existing key, use [sh_set](./sh_set).


```py
sh_add(tkn: int, keys: dict) -> None
```

|Param|Description|
|--|--|
|keys|Keys to store. If a key already exists, it does not overwrite|


## Raises
- `ResponseError` if query fails

