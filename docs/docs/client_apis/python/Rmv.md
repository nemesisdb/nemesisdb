---
sidebar_position: 5
displayed_sidebar: clientApisSidebar
sidebar_label: rmv
---

# rmv
Delete key(s).

- To delete all keys, use [clear](./Clear)
- You can use [clear_set](./Clear_Set.md) which clears all keys then sets new keys in a single command.


|Param|Type|Description|Required|
|--|:-:|--|:-:|
|keys|tuple|Keys to delete|Y|


## Returns
None


## Examples


```py title='Delete one key'
client = KvClient()
await client.open('ws://127.0.0.1:1987/')
# ...
await client.rmv(('username',))
```

<br/>

```py title='Delete multiple keys'
client = KvClient()
await client.open('ws://127.0.0.1:1987/')
# ...
await client.rmv(('key1','key2','key3'))
```