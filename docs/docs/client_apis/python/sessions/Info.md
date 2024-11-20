---
sidebar_position: 5
displayed_sidebar: clientApisSidebar
sidebar_label: session_info
---

# session_info
Returns information for a session.


## Returns


`tuple(bool, dict)`
- `bool` : `True` is the command was successful, otherwise `False`
- `dict` : information. See table below




|Param|Type|Meaning|
|:---|:---|:---|
|tkn|unsigned int|Session token|
|keyCnt|unsigned int|Number of keys in the session|
|shared|bool|Indicates if the session is shared|
|expires|bool|true if the session expires, false if it never expires|
|expiry|object|Expiry information, only present if `expires` is `true`|


`expiry`:

|Param|Type|Meaning|
|:---|:---|:---|
|duration|unsigned int|Duration, in seconds, before expiring|
|remaining|int|Duration, in seconds, remaining until the session expires. Note, this can be negative because the session can expire before the session monitor has checked|
|deleteSession|bool|If true, the session is deleted when the session expires. If false, only the data is deleted|

