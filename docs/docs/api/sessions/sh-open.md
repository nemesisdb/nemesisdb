---
sidebar_position: 10
---

# SH_OPEN
Opens a shared session from the session name.

The session must have been created with `"shared":true` for this command to succeed.

:::note
If the `name` is the same as an existing session but the existing session is not shared, this command fails.
:::


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|Session name. Must be the same as the name used when created.|Y|


## Response

`SH_OPEN_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|name|string|Session name as used in the request.|
|tkn|uint|Session token|
|st|unsigned int|Status|


Possible statuses:

- Ok
- SessionNotExist
- SessionOpenFail (`name` incorrect or session is not shared)
- ValueMissing (no `name`)
- ValueTypeInvalid (`name` is wrong type)
- ValueSize (`name` is empty)
