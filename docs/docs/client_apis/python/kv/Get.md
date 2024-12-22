---
sidebar_position: 3
displayed_sidebar: clientApisSidebar
sidebar_label: get
---

# get
```py
async def get(keys = None, key=None) -> dict | Any:
```

|Param|Description|
|--|--|
|key|Key to retrieve|
|keys|Keys to retrieve|


Retrieves keys from the database.

## Returns
- If `keys` set, a `dict` is returned
  - If a key does not exist, its value is `None`
- If value of `key` is returned
  - If `key` does not exist, `None` is returned


## Raises
- `ResponseError`
- `ValueError`
  - Both `keys` and `key` are set
- `TypeError`
  - `keys` not a dict
  - `key` not a str


## Examples

```py title='Connect and Set'
from ndb.client import NdbClient
from ndb.kv import KV


client = NdbClient(debug=False) # toggle for debug
await client.open('ws://127.0.0.1:1987/')

# create API instance
kv = KV(client)
await kv.set({'username':'billy', 'password':'billy_passy'})
```


```py title='Get various'
# get single key
value = await kv.get(key='username')
print (value)

# get multiple keys
values = await kv.get(keys=('username', 'password'))
print (values)
```

Output:
```
billy
{'password': 'billy_passy', 'username': 'billy'}
```
