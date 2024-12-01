---
sidebar_position: 40
displayed_sidebar: clientApisSidebar
---

# sh_add
Stores keys but does not overwrite a key if it already exists.

To overwrite an existing key, use [sh_set](./sh_set).

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|tuple|Tuple of keys to retrieve|Y|
|tkn|int|Session token|Y|

If a key already exists it is not considered an error.


## Returns
None



## Examples

```py title='Avoid overwriting'
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

await client.sh_set({'LinuxDistro':'Arch'})
values = await client.sh_get(('LinuxDistro',))
print(f'Before add(): {values}')

await client.sh_add({'LinuxDistro':'Arch btw'})
values = await client.sh_get(('LinuxDistro',))
print(f'After add(): {values}')

await client.sh_set({'LinuxDistro':'Arch btw'})
values = await client.sh_get(('LinuxDistro',))
print(f'After set(): {values}')
```

- `sh_add()` does not overwrite keys
- `sh_set()` does overwrite keys

Output:
```
Before add(): {'LinuxDistro': 'Arch'}
After add(): {'LinuxDistro': 'Arch'}
After set(): {'LinuxDistro': 'Arch btw'}
```