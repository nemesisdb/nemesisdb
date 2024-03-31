---
sidebar_position: 20
---

# KV_SETQ
Stores key-values. It is identical to `KV_SET` except it only sends a response on failure.

The purpose is to reduce network traffic, so it is only suitable if you don't need confirmation of success (i.e. you only need confirmation if it fails).

See [`KV_SET`](./kv-set) for structure.

<br/>

## Response

`KV_SETQ_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|uint|Status|


Possible status values:

- ParamMissing
- ValueTypeInvalid

See the [response status](./../Statuses) page for status values.

:::info
Because `KV_SETQ` only returns a response on error the `st` will never be `Ok` (`1`).
:::
