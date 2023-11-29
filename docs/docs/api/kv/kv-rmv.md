---
sidebar_position: 70
---

# KV_RMV
Removes one or multiple keys. If you want to remove all keys, use `KV_CLEAR`.

The command requires the token and an array of keys to remove. 


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|unsigned int|Session token|Y|
|keys|array|Array of keys to remove|Y|


## Response

`KV_RMV_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|unsigned int|Session token|

After the `tkn`, each key is listed with its removal status, typically KeyRemoved or KeyNotExist.

See the [response status](./../Statuses) page for status values.


### Remove Key

```json
{
  "KV_RMV":
  {
    "tkn":16127334958516145570,
    "keys":["blocked"]
  }
}
```

Response:

```json title="Remove success"
{
  "KV_RMV_RSP": {
    "tkn": 16127334958516145570,
    "blocked": 24
  }
}
```

### A Key Does Not Exist

```json
{
  "KV_RMV":
  {
    "tkn":16127334958516145570,
    "keys":["IDontExist"]
  }
}
```

```json title="Key Does Not Exist"
{
  "KV_RMV_RSP": {
    "tkn": 16127334958516145570,
    "IDontExist": 22
  }
}
```