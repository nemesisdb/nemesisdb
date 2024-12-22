---
sidebar_position: 12
displayed_sidebar: clientApisSidebar
sidebar_label: load
---

# load

```py
async def load(name: str) -> int
```

|Param|Description|
|--|--|
|name|The name of the dataset.<br/>The `name` was used previously with `save()`|

Restores keys that were previously persisted with `save()`.


:::note
- Persistance does _not_ have to be enabled for this command
- You can also load at startup, using `--loadName`
:::


## Returns
The number of keys loaded.


## Raises
- `ResponseError`
- `ValueError`
  - `name` is empty


## Examples

Assuming a previous call to `save('my_data')`:

```py title='Load keys'
count = await kv.load('my_data')
print(f'Loaded {count} keys')
```
