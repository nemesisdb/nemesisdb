---
sidebar_position: 12
displayed_sidebar: clientApisSidebar
sidebar_label: load
---

# load
Restores keys that were previously saved with `save()`.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|name|str|The name of the dataset|Y|


:::note
- Persistance does _not_ have to be enabled for this command
- This command can only be used with sessions are disabled. With sessions enabled, use `SessionClient.load()`.
:::


## Returns

`tuple(bool,int)`:
- `bool` : `True` if command successful, otherwise `False`
- `int` : The number of keys loaded


## Examples


Assuming a previous call to `save('my_data')`:
```py
(loaded, count) = await client.load('my_data')
if loaded:
  print(f'Loaded {count} keys')
```
