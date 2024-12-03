---
sidebar_position: 90
displayed_sidebar: clientApisSidebar
sidebar_label: sh_contains
---

# sh_contains
Given a tuple of keys, returns the keys which exist in the session.

```py
sh_contains(tkn: int, keys: tuple) -> List[str]:
```


## Returns
The keys that exist


## Raises
- `ResponseError` if query fails


## Examples

```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()

await client.sh_set(session.tkn, {'k1':10, 'k2':20})

exist = await client.sh_contains(session.tkn, ('k1','k2','k3'))
print(exist) # ['k1', 'k2']
```
