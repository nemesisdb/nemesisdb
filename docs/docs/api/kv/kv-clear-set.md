---
sidebar_position: 85
---

# KV_CLEAR_SET
Removes all keys from the session and set new keys in a single command.

This command is an alternative to sending separate `KV_CLEAR` and `KV_SET` requests.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|unsigned int|Session token|Y|
|keys|object|Keys and values to store, same structure as `keys` in `KV_SET`|Y|

<br/>

## Response

`KV_CLEAR_SET_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|unsigned int|Session token|
|st|unsigned int|Status of the clear operation|
|cnt|unsigned int|Number of keys deleted during clear. <br/>If there was an error during the clear, `cnt` is 0.|
|keys|object|Same as `keys` in `KV_SET`. <br/> If there was an error during the clear, `keys` is empty.|

<br/>

The status (`st`) can be either:

- Ok
- Unknown

If the status is Unknown, the clear failed and all other values should be ignored.


<br/>

## Example

```json title="Clear session and set new keys"
{
  "KV_CLEAR_SET":
  {
    "tkn":1821853096589685818,
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
    "tkn": 1821853096589685818,
    "st": 1,
    "cnt": 2,
    "keys":
    {
      "username": 20,
      "movieNames": 20,
      "yearBorn": 20
    }
  }
}
```
