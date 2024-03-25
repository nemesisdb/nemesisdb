---
sidebar_position: 90
---

# KV_COUNT
Returns how many keys are in a session.


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

```json title="Map contains 3,457 keys"
{
  "KV_COUNT_RSP":
  {
    "st": 1,
    "cnt": 3457
  }
}
```
