---
sidebar_position: 60
---

# KV_CONTAINS
Check if a key exists.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|keys|array|Array of key names|Y|


```json
{
  "KV_CONTAINS":
  {
    "keys":["user", "access", "IDontExist"]
  }
}
```

## Response

`KV_CONTAINS_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|contains|array|Array of key names that do exist|

If a key does not exist, it is not present in `contains`.

Possible status values:

- Ok
- ParamMissing
- ValueTypeInvalid

See the [response status](./../Statuses) page for status values.



```json title="Only user and access exist"
{
  "KV_CONTAINS":
  {
    "keys":["user", "access", "nothere"]
  }
}
```


```json title="Key nothere does not exist"
{
  "KV_CONTAINS_RSP":
  {
    "st": 1,
    "contains":["user", "access"]
  }
}
```