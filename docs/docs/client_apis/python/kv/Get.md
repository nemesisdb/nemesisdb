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

|Param|Description|Returns|
|--|--|--|
|key|Key to return|Any (type of value)
|keys|Keys to retrieve|dict|


## Raises
- `ResponseError` if query fails
- `ValueError` if both `keys` and `key` are set


## Examples

```py title='Connect'
from ndb.client import NdbClient

client = NdbClient(debug=False) # toggle for debug
await client.open('ws://127.0.0.1:1987/')
```


```py title='Set various'
# get single key
await client.kv_set({'username':'billy', 'password':'billy_passy'})
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
