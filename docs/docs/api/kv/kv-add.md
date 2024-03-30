---
sidebar_position: 30
---

# KV_ADD
Stores key-values if the key does not already exist.

This command is the same as `KV_SET` but it will not overwrite an existing key.

If `keys` contains a key that already exists it is not considered an error. The key's existing value is not changed.

## Response

`KV_ADD_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|

<br/>


Possible status values:

- ParamMissing
- ValueTypeInvalid
