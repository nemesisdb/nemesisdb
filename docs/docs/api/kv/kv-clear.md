---
sidebar_position: 80
---

# KV_CLEAR
Removes all keys from the session. If you want to remove keys individually, use `KV_RMV`.


## Response

`KV_CLEAR_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|cnt|unsigned int|Number of keys cleared|

<br/>

:::info
The `tkn` must be set when the server mode is "kv_sessions".
:::

Possible status values:

- Ok
- CommandSyntax



```json
{
  "KV_CLEAR":
  {
  }
}
```

Response:

```json title="Cleared session which contained two keys"
{
  "KV_CLEAR_RSP": {
    "st": 1,
    "cnt": 2
  }
}
```
