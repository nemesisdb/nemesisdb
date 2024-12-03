---
sidebar_position: 5
displayed_sidebar: clientApisSidebar
sidebar_label: kv_rmv
---

# kv_rmv
Delete particular key(s).

- To delete all keys, use [kv_clear](./Clear)
- You can use [kv_clear_set](./Clear_Set.md) which clears all keys then sets new keys in a single command.


```py
kv_rmv(keys: tuple)
```

|Param|Description|Returns|
|--|--|:--:|
|keys|Keys to clear|None|



## Raises
- `ResponseError` if query fails
- `ValueError` if keys is empty


## Examples


```py title='Delete one key'
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')
# set keys ...
await client.kv_rmv(('username',))
```

<br/>

```py title='Delete multiple keys'
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')
# set keys ...
await client.kv_rmv(('key1','key2','key3'))
```