---
sidebar_position: 3
displayed_sidebar: clientApisSidebar
sidebar_label: kv_get
---

# kv_get
Retrieves keys from the database.

```py
kv_get(keys = tuple(), key = None)
```

|Param|Description|
|--|--|
|key|Key to retrieve|
|keys|Keys to retrieve|


## Returns
- If `keys` set, a `dict` is returned
- Otherwise, the value of the `key` is returned


## Raises
- `ResponseError` if query fails
- `ValueError` if both `keys` and `key` are set


## Examples

```py title='Connect and Set'
from ndb.client import NdbClient

client = NdbClient(debug=False) # toggle for debug

await client.open('ws://127.0.0.1:1987/')
await client.kv_set({'username':'billy', 'password':'billy_passy'})
```


```py title='Get various'
# get single key
value = await client.kv_get(key='username')
print (value)

# get multiple keys
values = await client.kv_get(keys=('username', 'password'))
print (values)
```

Output:
```
billy
{'password': 'billy_passy', 'username': 'billy'}
```
