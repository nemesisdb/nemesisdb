---
sidebar_position: 60
---

# KV_CONTAINS
Check if a key exists.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|uint|Session token|Y|
|keys|array|Array of key names|Y|


```json
{
  "KV_CONTAINS":
  {
    "tkn":"10388228285655098522",
    "keys":["user", "access", "IDontExist"]
  }
}
```

## Response

`KV_CONTAINS_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|uint|Session token|
|st|unsigned int|Status|
|keys|object|Keys with boolean flag indicating `true` (exist) or `false` (not exist)|Y|


Possible status values:

- Ok
- ParamMissing (no `keys`)
- ValueTypeInvalid (`keys` not an array)

See the [response status](./../Statuses) page for status values.


The above command may return this:

```json title="A key does not exist"
{
  "KV_CONTAINS_RSP":
  {
    "st": 1,
    "tkn": 10388228285655098522,
    "keys": {
      "user": true,
      "access": true,
      "IDontExist": false
    }
  }
}
```