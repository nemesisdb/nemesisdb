---
sidebar_position: 90
---

# KV_COUNT
Returns the number of keys present:

- With sessions enabled, count is the number of keys in the session
- With sessions disabled, count is the total number of keys


## Response

`KV_COUNT_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|cnt|unsigned int|Number of keys in the session|


Possible status values:

- Ok
- CommandSyntax


Request: 

```json
{
  "KV_COUNT":{}
}
```

Response:

```json title="Database contains 3,457 keys"
{
  "KV_COUNT_RSP":
  {
    "st": 1,
    "cnt": 3457
  }
}
```
