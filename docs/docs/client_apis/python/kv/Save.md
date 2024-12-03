---
sidebar_position: 11
displayed_sidebar: clientApisSidebar
sidebar_label: kv_save
---

# kv_save
Saves all keys to the filesystem so they can be restored later.


```py
kv_save(name: str) -> None
```

|Param|Description|
|--|--|
|name|The name of the dataset.<br/>The `name` is used to load data at runtime with `kv_load()` or at startup.|


:::note
- Persistance must be enabled in the server config
- To persist sessions, use [sh_save()](../sh/Save)
:::


## Returns
`None`


## Raises
- `ValueError` if name is empty
- `ResponseError` if query fails




## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# set keys, etc

await client.kv_save('my_data')
print('Save success')  
```

If the server is shutdown, we can restore the keys at runtime:

```py
count = await client.kv_load('my_data')
print(f'Loaded {count} keys')
```
