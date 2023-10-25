---
sidebar_position: 80
---

# KV_COUNT
Returns how many keys are in a session.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|string|Session token|Y|


## Response

`KV_COUNT_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|string|Session token|
|st|unsigned int|Status|
|cnt|unsigned int|Number of keys in the session|


Possible status values:

- Ok
- CommandSyntax


```json
{
  "KV_COUNT":
  {
    "tkn":"16127334958516145570"
  }
}
```

Response:

```json title="Cleared session which contained two keys"
{
  "KV_COUNT_RSP": {
    "st": 1,
    "cnt": 3457,
    "tkn": "16127334958516145570"
  }
}
```
