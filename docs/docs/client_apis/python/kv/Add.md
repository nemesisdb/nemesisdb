---
sidebar_position: 4
displayed_sidebar: clientApisSidebar
sidebar_label: add
---

# add

```py
async def add(keys: dict) -> None
```

|Param|Description|Returns|
|--|--|:--:|
|keys|The key/values to store|None|


Stores keys but does not overwrite a key if it already exists.

To overwrite an existing key, use [set](./Set).

If a key already exists, no changes are made and it is not considered an error.



## Raises
- `ResponseError`


## Examples

```py title='Avoid overwriting'
await kv.set({'LinuxDistro':'Arch'})
value = await kv.get(key='LinuxDistro')
print(f'Before add(): {value}')

# does not overwrite
await kv.add({'LinuxDistro':'Arch btw'})
value = await kv.get(key='LinuxDistro')
print(f'After add(): {value}')

# overwrite
await kv.set({'LinuxDistro':'Arch btw'})
value = await kv.get(key='LinuxDistro')
print(f'After set(): {value}')
```

Output:
```
Before add(): {'LinuxDistro': 'Arch'}
After add(): {'LinuxDistro': 'Arch'}
After set(): {'LinuxDistro': 'Arch btw'}
```

- `add()` does not overwrite keys
- `set()` does overwrite keys