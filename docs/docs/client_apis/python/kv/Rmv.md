---
sidebar_position: 5
displayed_sidebar: clientApisSidebar
sidebar_label: rmv
---

# rmv
```py
async def rmv(keys: tuple) -> None
```

|Param|Description|
|--|--|:--:|
|keys|Keys to delete|


Delete particular key(s).

- To delete all keys, use [clear](./Clear)
- You can use [clear_set](./Clear_Set.md) which deletes all keys then sets new keys in a single command.


## Raises
- `ResponseError`
- `ValueError`
  - `keys` is empty


## Examples

```py title='Delete one key'
# ... set keys ...

await kv.rmv(('username',))
```

<br/>

```py title='Delete multiple keys'
# ... set keys ...

await kv.rmv(('key1','key2','key3'))
```