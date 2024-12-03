---
sidebar_position: 4
displayed_sidebar: clientApisSidebar
sidebar_label: kv_add
---

# kv_add
Stores keys but does not overwrite a key if it already exists.

To overwrite an existing key, use [kv_set](./Set).


```py
kv_add(keys: dict)
```

|Param|Description|Returns|
|--|--|:--:|
|keys|The key/values to store|None|



## Raises
- `ResponseError` if query fails


## Examples

```py title='Avoid overwriting'
await client.kv_set({'LinuxDistro':'Arch'})
value = await client.kv_get(key='LinuxDistro')
print(f'Before add(): {value}')

# kv_add() does not overwrite
await client.kv_add({'LinuxDistro':'Arch btw'})
value = await client.kv_get(key='LinuxDistro')
print(f'After add(): {value}')

# kv_set() does overwrite
await client.kv_set({'LinuxDistro':'Arch btw'})
value = await client.kv_get(key='LinuxDistro')
print(f'After set(): {value}')
```

- `kv_add()` does not overwrite keys
- `kv_set()` does overwrite keys

Output:
```
Before add(): {'LinuxDistro': 'Arch'}
After add(): {'LinuxDistro': 'Arch'}
After set(): {'LinuxDistro': 'Arch btw'}
```