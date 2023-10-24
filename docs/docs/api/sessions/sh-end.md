---
sidebar_position: 2
---

# SH_END
Ends a session, deleting the session data.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|string|Session token|Y|


## Ending a session before it expires

If `SH_END` ends a session which has expiry settings, those settings are ignored and session and its data are always deleted immediately, even if the `"deleteSession":false` was set in `SH_NEW`.


## Response

`SH_END_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|string|Session token|
|st|unsigned int|Status|


Possible status values:

- Ok
- SessionNotExist
- SessionTokenInvalid 
