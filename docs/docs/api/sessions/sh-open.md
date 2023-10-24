---
sidebar_position: 3
---

# SH_OPEN
Opens a shared session from the session name.

The session must have been created with `"shared":true` for this command to succeed.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|Session name. Must be the same as the name used when created.|Y|


## Response

`SH_OPEN_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|name|string|Session name as used in the request.|
|tkn|string|Session token|
|st|unsigned int|Status|


Possible statuses:

- Ok
- SessionNotExist
- SessionNotShared
