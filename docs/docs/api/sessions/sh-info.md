---
sidebar_position: 4
---

# SH_INFO
Get session information.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|string|Session token|Y|


## Response

`SH_INFO_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|tkn|string|Session token|
|keyCnt|unsigned int|Number of keys in the session|
|shared|bool|Indicates if the session is shared|
|expiry|object|Expiry information|


`expiry`:

|Param|Type|Meaning|
|:---|:---|:---|
|expires|bool|true if the session expires, false is it never expires|
|duration|unsigned int|Time, in seconds, the session can exist|
|time|int|Timestamp of expiry time. This timestamp is in seconds|
|deleteSession|bool|If true, the session is deleted when the session expires. If false, only the data is deleted|


Possible status values:

- Ok
- SessionNotExist
- SessionTokenInvalid 
- CommandSyntax