---
sidebar_position: 11
displayed_sidebar: clientApisSidebar
sidebar_label: save
---

# save
```py
async def save(name: str) -> None
```

|Param|Description|
|--|--|
|name|The name of the dataset.<br/>The `name` is used to load data at runtime with `load()` or at startup.|


Saves all keys to the filesystem so they can be restored later.


:::note
- Persistance must be enabled in the server config
:::


## Returns
`None`


## Raises
- `ResponseError` if query fails
- `ValueError`
  - `name` is empty


## Examples

```py
# set keys ... etc

await kv.save('my_data')
print('Save success')  
```

If the server is shutdown, we can restore the keys at runtime:

```py
count = await kv.load('my_data')
print(f'Loaded {count} keys')
```
