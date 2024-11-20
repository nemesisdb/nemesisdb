---
sidebar_position: 11
displayed_sidebar: clientApisSidebar
sidebar_label: save
---

# save
Saves all keys to the filesystem so they can be restored later.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|name|str|The name of the dataset|Y|

The `name` is used to load data at runtime with `KV_LOAD` and at startup:


:::note
- Persistance must be enabled in the server config
- With sessions enabled, use [session_save()](./sessions/Save)
:::


## Returns

`bool`: `True` if command successful, otherwise `False`


## Examples


```py
client = KvClient()
await client.open('ws://127.0.0.1:1987/')

# set keys, etc
saved = await client.save('my_data')
if saved:
  print('Save success')  
```

If the server is shutdown, we can restore the keys:

```py
(loaded, count) = await client.load('my_data')
if loaded:
  print(f'Loaded {count} keys')
```
