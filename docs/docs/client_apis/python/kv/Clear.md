---
sidebar_position: 6
displayed_sidebar: clientApisSidebar
sidebar_label: clear
---

# clear

```py
async def clear() -> int
```


Deletes _all_ keys.

- To delete select key(s) use [rmv](./Rmv)
- To delete all keys and set new keys in a single call, use [clear_set](./Clear_Set)


# Returns
The number of keys deleted


## Raises
- `ResponseError`



## Examples

```py
count = await kv.clear()
print(f'Deleted {count} keys')
```