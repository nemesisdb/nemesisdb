---
sidebar_position: 60
displayed_sidebar: clientApisSidebar
sidebar_label: sh_clear
---

# sh_clear
Deletes _all_ keys from a session.

- To delete particular key(s) use [sh_rmv](./sh_rmv)
- To delete all keys and set new keys in a single call, use [sh_clear_set](./sh_clear_set)

```py
sh_clear(tkn: int) -> int:
```


## Returns
The number of keys cleared



## Raises
- `ResponseError` if query fails


## Examples

```py
# connect, create session and store keys ..

count = await client.sh_clear(session.tkn)
print(f'Cleared {count} keys')
```