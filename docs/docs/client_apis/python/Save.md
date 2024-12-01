---
sidebar_position: 11
displayed_sidebar: clientApisSidebar
sidebar_label: save
---

# kv_save
Saves all keys to the filesystem so they can be restored later.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|name|str|The name of the dataset|Y|

The `name` is used to load data at runtime with `kv_load()` and at startup.


:::note
- Persistance must be enabled in the server config
- To persist sessions, use [sh_save()](./sessions/Save)
:::


## Returns
None


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
