---
sidebar_position: 12
displayed_sidebar: clientApisSidebar
sidebar_label: kv_load
---

# kv_load
Restores keys that were previously saved with `save()`.

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|name|str|The name of the dataset|Y|


:::note
- Persistance does _not_ have to be enabled for this command
- You can also load at startup, using `--loadName`
- This command can only be used to load independent keys (those that are not in a session). To load sessions, use [sh_load()](../sh/Load)
:::


## Returns
`int` : The number of keys loaded


## Examples

Assuming a previous call to `kv_save('my_data')`:

```py title='Load keys'
count = await client.kv_load('my_data')
print(f'Loaded {count} keys')
```
