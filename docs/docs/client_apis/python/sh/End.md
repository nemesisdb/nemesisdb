---
sidebar_position: 2
displayed_sidebar: clientApisSidebar
sidebar_label: sh_end
---

# sh_end
Ends a session.

The session is ended immediately, with all keys being deleted.


```py
sh_end(tkn: int)() -> None
```



## Examples


```py
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create()

await client.set(session.tkn, {'key1':'value1'})

# ... use session ...

# end session
await client.end(session.tkn)
```
