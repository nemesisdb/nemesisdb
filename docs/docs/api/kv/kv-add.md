---
sidebar_position: 30
---

# KV_ADD
Stores key-values if the key does not already exist.

This command is the same as `KV_SET` but it will not overwrite an existing key.


## Response

`KV_ADD_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|unsigned int|Session token|
|keys|object|For each key that was set: `"<keyname>":<status>`|


Possible status values:

- KeySet
- KeyExists
- ParamMissing (no `keys`)
- ValueTypeInvalid (`keys` not an object)
