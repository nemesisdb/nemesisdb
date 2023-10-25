---
sidebar_position: 40
---

# KV_ADDQ
Stores data. This command is the same as `KV_ADD` but it will only response on an error (i.e. the key already exists).

This is most useful when you are initially populating an empty database so keys can't already exist.


## Response

`KV_ADDQ_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|string|Session token|
|keys|object|For each key that was set: `"<keyname>":<status>`|


Possible status values:

- KeyExists
- ParamMissing (no `keys`)
- ValueTypeInvalid (`keys` not an object)



:::info
Because `KV_ADDQ` only returns a response on error the `st` will never be `KeySet` (`20`).
:::
