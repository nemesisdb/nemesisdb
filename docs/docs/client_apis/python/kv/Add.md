---
sidebar_position: 4
displayed_sidebar: clientApisSidebar
sidebar_label: kv_add
---

# kv_add
Stores keys but does not overwrite a key if it already exists.

To overwrite an existing key, use [kv_set](./Set).

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|tuple|Tuple of keys to retrieve|Y|


If a key already exists it is not considered an error.


## Returns
None



## Examples

```py title='Avoid overwriting'
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

await client.kv_set({'LinuxDistro':'Arch'})
values = await client.kv_get(('LinuxDistro',))
print(f'Before add(): {values}')

await client.kv_add({'LinuxDistro':'Arch btw'})
values = await client.kv_get(('LinuxDistro',))
print(f'After add(): {values}')

await client.kv_set({'LinuxDistro':'Arch btw'})
values = await client.kv_get(('LinuxDistro',))
print(f'After set(): {values}')
```

- `kv_add()` does not overwrite keys
- `kv_set()` does overwrite keys

Output:
```
Before add(): {'LinuxDistro': 'Arch'}
After add(): {'LinuxDistro': 'Arch'}
After set(): {'LinuxDistro': 'Arch btw'}
```