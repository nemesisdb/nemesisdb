---
sidebar_position: 6
displayed_sidebar: clientApisSidebar
sidebar_label: kv_clear
---

# kv_clear
Deletes _all_ keys.


- To delete select key(s) use [kv_rmv](./Rmv)
- To delete all keys and set new keys in a single call, use [kv_clear_set](./Clear_Set)


```py
kv_clear()
```

# Returns
`int` - the number of keys deleted


## Raises
- `ResponseError` if query fails
- `ValueError` if keys is empty



## Examples

```py
count = await client.kv_clear()
print(f'Cleared {count} keys')
```