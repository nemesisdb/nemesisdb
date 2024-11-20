---
sidebar_position: 4
displayed_sidebar: clientApisSidebar
sidebar_label: add
---

# add
Stores keys but does not overwrite a key if it already exists.

To overwrite an existing key, use [set](./Set).

|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|tuple|Tuple of keys to retrieve|Y|
|tkn|int|A session token|Only if sessions enabled|


## Returns

`bool`
- `True` if command successful, otherwise `False`

If a key already exists it is not considered an error.


## Examples

```py title='Avoid overwriting'
client = KvClient()
await client.open('ws://127.0.0.1:1987/')

await client.set({'LinuxDistro':'Arch'})
(getOk, values) = await client.get(('LinuxDistro',))
print(f'Before add(): {values}')

await client.add({'LinuxDistro':'Arch btw'})
(getOk, values) = await client.get(('LinuxDistro',))
print(f'After add(): {values}')

await client.set({'LinuxDistro':'Arch btw'})
(getOk, values) = await client.get(('LinuxDistro',))
print(f'After set(): {values}')
```

- calling `add()` does not overwrite the key
- use `set()` to replace the value

Output:
```
Before add(): {'LinuxDistro': 'Arch'}
After add(): {'LinuxDistro': 'Arch'}
After set(): {'LinuxDistro': 'Arch btw'}
```