---
sidebar_position: 80
---

# KV_CLEAR
Removes all keys from the session. If you want to remove keys individually, use `KV_RMV`.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|string|Session token|Y|


## Response

`KV_CLEAR_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|string|Session token|
|st|unsigned int|Status|
|cnt|unsigned int|Number of keys cleared|


Possible status values:

- Ok
- CommandSyntax



```json
{
  "KV_CLEAR":
  {
    "tkn":"16127334958516145570"
  }
}
```

Response:

```json title="Cleared session which contained two keys"
{
  "KV_CLEAR_RSP": {
    "st": 1,
    "cnt": 2,
    "tkn": "16127334958516145570"
  }
}
```