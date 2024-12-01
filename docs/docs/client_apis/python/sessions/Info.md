---
sidebar_position: 5
displayed_sidebar: clientApisSidebar
sidebar_label: sh_info
---

# sh_info
Returns information for a session.


|Param|Type|Description|Required|
|--|:-:|--|:-:|
|tkn|int|The token|Y|


## Returns
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
|deleteSession| bool|`true`: session is deleted when it expires<br/>`false`: only the keys are deleted (default)|
|extendOnSetAdd|bool|`true`: on each set or add, the expire time is extended by `duration`<br/>`false`: default|
|extendOnGet|bool|`true`: on each get, the expire time is extended by `duration`<br/>`false`: default|

