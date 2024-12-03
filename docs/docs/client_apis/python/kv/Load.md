---
sidebar_position: 12
displayed_sidebar: clientApisSidebar
sidebar_label: kv_load
---

# kv_load
Restores keys that were previously saved with `kv_save()`.


```py
kv_load(name: str)
```

|Param|Description|
|--|--|
|name|The name of the dataset.<br/>The `name` was used previously with `kv_save()`|


:::note
- Persistance does _not_ have to be enabled for this command
- You can also load at startup, using `--loadName`
- To load sessions, use [sh_load()](../sh/Load)
:::


## Returns
`int` : The number of keys loaded


## Raises
- `ValueError` if name is empty
- `ResponseError` if query fails


## Examples

Assuming a previous call to `kv_save('my_data')`:

```py title='Load keys'
count = await client.kv_load('my_data')
print(f'Loaded {count} keys')
```
