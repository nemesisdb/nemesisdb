---
sidebar_position: 1
displayed_sidebar: apiSidebar
---

# Response Status Values
Many commands return a status (`st`) value which is an unsigned integer.

This table lists a 'friendly name' and value. The friendly name is used throughout the docs rather than the number value for readability and in case of changes.


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
|CommandDisabled|14|Command has been disabled (only applies to `SH_SAVE`)|
|ParamMissing|26|A param is not in the command|


## Session
|Name|Value|Meaning
|:---|:---:|:---|
|SessionNotExist|100|Session does not exist|
|SessionTokenInvalid|101|Token is not an unsigned int, not present or null|
|SessionOpenFail|102|Failed to open session, either the name is incorrect or the session is not shared|
|SessionNewFail|103|Failed to create new session. Only likely when creating a shared session with a name for a shared session that already exists|


## Keys
|Name|Value|Meaning
|:---|:---:|:---|
|KeySet|20|Key value is set.|
|KeyUpdated|21|Key already existed and has been updated|
|KeyNotExist|22|Key does not exist|
|KeyExist|23|Key exists (i.e. with `KV_CONTAINS`)|
|KeyRemoved|24|Key deleted/removed|
|KeyTypeInvalid|27|Key wrong type. Must always be a string|


## Values
|Name|Value|Meaning
|:---|:---:|:---|
|ValueMissing|40|Expected value not present|
|ValueTypeInvalid|41|Value has incorrect type|
|ValueSize|42|Value exceeds maximum size <br/> NOTE: if the whole WebSocket message payload size exceeds the maximum, a different response is returned|


## Save
|Name|Value|Meaning
|:---|:---:|:---|
|SaveStart|120|`SH_SAVE` accepted and writing data begins|
|SaveComplete|121|`SH_SAVE` finished without error|
|SaveDirWriteFail|122|Could not create directories/files whilst preparing `SH_SAVE` (no data written)|
|SaveError|123|Could not complete `SH_SAVE`, this can be received after SaveStart|


## Load
|Name|Value|Meaning
|:---|:---:|:---|
|LoadComplete|141|Session(s) loaded without error|
|LoadError|142|Error during load|
|LoadDuplicate|143|Load success with duplicate sessions|
