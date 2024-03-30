---
sidebar_position: 85
---

# KV_CLEAR_SET
Removes all keys and set new keys in a single command.

This command is an alternative to sending separate `KV_CLEAR` and `KV_SET` requests.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|keys|object|Keys and values to store, same structure as `keys` in `KV_SET`|Y|

<br/>

## Response

`KV_CLEAR_SET_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status of the clear operation|
|cnt|unsigned int|Number of keys deleted during clear. <br/>If there was an error during the clear, `cnt` is 0.|

<br/>

:::info
The `tkn` must be set when the server mode is "kv_sessions".
:::

<br/>

The status (`st`) can be either:

- Ok
- Unknown


<br/>

## Example

```json title="Clear and set new keys"
{
  "KV_CLEAR_SET":
  {
    "keys":
    {
      "username":"The Rock",
      "movieNames":["Jumanji","Hercules"],
      "yearBorn":1972
    }
  }
}
```

Response:

```json title="Response includes status for each key set"
{
  "KV_CLEAR_SET_RSP":
  {
    "st": 1,
    "cnt": 2
  }
}
```
