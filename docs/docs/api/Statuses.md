---
sidebar_position: 1
displayed_sidebar: apiSidebar
---

# Response Status Values
Many commands return a status (`st`) value which is an unsigned integer.

This table listed a 'friendly name' and value. THe friendly name is used throughout the API docs rather than the number value for readability.


## General
|Name|Value|Meaning
|:---|:---:|:---|
|Ok|1|Command successful, everything good|
|PayLoadInvalid|2|WebSocket pay invalid, must be text|
|JsonInvalid|3|JSON failed parsing|
|PathInvalid|4|Path invalid|
|NoPath|5|No `path`|
|CommandNotExist|10|Command does not exist|
|CommandMultiple|11|Multiple commands in the same request|
|CommandType|12|Command is known but it is the incorrect type (most commands are objects)|
|CommandSyntax|13|Command is known but contains invalid syntax. `m` will contain the offending parameter if known.|
|ParamMissing|26|A param is not in the command|


## Keys
|Name|Value|Meaning
|:---|:---:|:---|
|KeySet|20|Key value is set.|
|KeyUpdated|21|Key already existed and has been updated|
|KeyNotExist|22|Key does not exist|
|KeyExist|23|Key exist (i.e. with `KV_CONTAINS`)|
|KeyRemoved|24|Key deleted/removed|
|KeyTypeInvalid|27|Key wrong type. Must always be a string|


## Values
|Name|Value|Meaning
|:---|:---:|:---|
|ValueMissing|40|Expected value not present|
|ValueTypeInvalid|41|Value has incorrect type|
|ValueSize|42|Value exceeds maximum size (NOTE: if the payload size exceeds the maximum, this is not returned)|


## Session
|Name|Value|Meaning
|:---|:---:|:---|
|SessionNotExist|100|Session does not exist|
|SessionTokenInvalid|101|Token is not a string, not present or empty|
|SessionOpenFail|102|Failed to open session, either the name is incorrect or the session is not shared|
|SessionNewFail|103|Failed to create new session. Only likely when creating a shared session with a name for a shared session that already exists|

