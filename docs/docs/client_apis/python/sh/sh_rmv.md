---
sidebar_position: 50
displayed_sidebar: clientApisSidebar
sidebar_label: sh_rmv
---

# sh_rmv
Delete particular key(s) from a session.

- Use [sh_clear](./sh_clear) to delete all keys in the session
- Use [sh_clear_set](./sh_clear_set.md) to clear all keys and set new keys in a single command

```py
sh_rmv(tkn: int, keys: tuple) -> None:
```

|Param|Description|
|--|--|
|keys|Keys to remove. If a key does not exist, it is not an error|



## Raises
- `ResponseError` if query fails


