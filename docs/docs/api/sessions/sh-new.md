---
sidebar_position: 1
---

# SH_NEW
Creates a new session.

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|A session name. May be used when creating session token for shared sessions|Y|
|shared|bool|If true, the session token is created so that the token can be retrieved by the token name|Y|
|expiry|object|Defines session expiry settings. See below.|N|


Expiry

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|duration| unsigned int|Time in seconds until the session expires |Y|
|deleteSession| bool|Flag indicating if the session should be deleted. If `false`, only the data is deleted|Y|

<br/>

## Shared Sessions
Shared sessions don't refer to authentication - a session's data can be accessed by any client using the session token, there's no session authentication.

But that means clients knowing the session token. This may not be possible or practical because all services using the shared session have have the token, which is only known when the session is created, and then has to be sent to the other services.

A shared service helps by using the `name` to generate the session tkn. Other clients then use `SH_OPEN` with the same session name and receive the same session token.

<br/>

This can be useful if a session is used to store default/initial data. For example, default UI settings or access rights. Each service can get these from the shared 'defaults' session.

After initial deployment/update, the shared session is created, default data set then services call `SH_OPEN` with a known session name to access the data.

<center>
<font size="8">**PIC**</font>
</center>