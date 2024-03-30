---
sidebar_position: 70
---

# KV_RMV
Removes one or multiple keys. If you want to remove all keys, use `KV_CLEAR`.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|keys|array|Keys to remove|Y|

<br/>

:::info
The `tkn` must be set when the server mode is "kv_sessions".
:::

<br/>

## Response

`KV_RMV_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|


<br/>

If `keys` contains a key that does not exist it is ignored.


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