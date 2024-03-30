---
sidebar_position: 40
---

# KV_ADDQ
Stores data. This command is the same as `KV_ADD` but it will only send a response if an error occurs.

If `keys` contains a key that already exists it is not considered an error. The key's existing value is not changed.

## Response

`KV_ADDQ_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|

<br/>

:::info
The `tkn` must be set when the server mode is "kv_sessions".
:::

Possible status values:

- KeyExists
- ParamMissing
- ValueTypeInvalid

:::info
Because `KV_ADDQ` only returns a response on error, `st` can never be `Ok` (`1`).
:::
