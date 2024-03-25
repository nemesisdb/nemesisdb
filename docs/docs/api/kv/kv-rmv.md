---
sidebar_position: 70
---

# KV_RMV
Removes one or multiple keys. If you want to remove all keys, use `KV_CLEAR`.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|keys|array|Array of keys to remove|Y|


## Response

`KV_RMV_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|


<br/>

It is not an error if `keys` contains a key that does not exist.


Possible status:
  - Ok
  - Unknown

See the [response status](./../Statuses) page for status values.


## Example

```json
{
  "KV_RMV":
  {
    "keys":["user_1_access"]
  }
}
```

```json title="Response"
{
  "KV_RMV_RSP":
  {
    "st": 1
  }
}
```