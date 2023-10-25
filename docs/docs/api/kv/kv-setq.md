---
sidebar_position: 20
---

# KV_SETQ
Stores data. This is identical to `KV_SET` but it only sends a response on failure.

The purpose of this command is reduce network traffic. It is only suitable if you don't need confirmation the command is successful (or you only need confirmation if it wasn't successful).


See [`KV_SET`](./kv-set) for structure.

<br/>


## Response

`KV_SETQ_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|string|Session token|
|keys|object|For each key that was set: `"<keyname>":<status>`|


Possible status values:

- ParamMissing (no `keys`)
- ValueTypeInvalid (`keys` not an object)


:::info
Because `KV_SETQ` only returns a response on error the `st` will never be `KeySet` (`20`).
:::
