---
sidebar_position: 50
---

# KV_GET
Get one or multiple keys.

The command requires the token and an array of keys to retrieve. 


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|string|Session token|Y|
|keys|array|Array of keys to retrieve|Y|


## Response

`KV_GET_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|string|Session token|
|keys|object|The keys and values requested|Y|

If a key does not exist, its value is returned as `null`.

Possible status values:

- KeyNotExists
- ParamMissing (no `keys`)
- ValueTypeInvalid (`keys` not an array)

See the [response status](./../Statuses) page for status values.


### All Keys Exist

```json
{
  "KV_GET":
  {
    "tkn":"5976385796811589752",
    "keys":["user", "blocked"]
  }
}
```

Response:

```json
{
  "KV_GET_RSP": {
    "tkn": "5976385796811589752",
    "keys": {
      "user": {
        "username": "Potato",
        "email": "spud@email.com",
        "avatar": "path/to/img.png"
      },
      "blocked": false
    }
  }
}
```

### A Key Does Not Exist

```json
{
  "KV_GET":
  {
    "tkn":"5976385796811589752",
    "keys":["blocked", "IDontExist"]
  }
}
```

```json
{
  "KV_GET_RSP": {
    "tkn": "5976385796811589752",
    "keys": {
      "blocked": false,
      "IDontExist": null
    }
  }
}
```