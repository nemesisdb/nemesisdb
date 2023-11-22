---
sidebar_position: 15
---

# SH_INFO
Get session information.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|uint|Session token|Y|


## Response

`SH_INFO_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|tkn|uint|Session token|
|keyCnt|unsigned int|Number of keys in the session|
|shared|bool|Indicates if the session is shared|
|expires|bool|true if the session expires, false if it never expires|
|expiry|object|Expiry information, only present if `expires` is `true`|


`expiry`:

|Param|Type|Meaning|
|:---|:---|:---|
|remaining|int|Duration, in seconds, until the session ends. Note, this can be negative because the session can end before the session monitor checks|
|duration|unsigned int|Duration, in seconds, the session exists before expiring|
|deleteSession|bool|If true, the session is deleted when the session expires. If false, only the data is deleted|


Possible status values:

- Ok
- SessionNotExist
- SessionTokenInvalid 
- CommandSyntax