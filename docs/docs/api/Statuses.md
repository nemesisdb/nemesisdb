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
|PathInvalid|4|Path invalid, for example in `KV_UPDATE`|
|PathNotExist|5|Path is valid syntax, but not found|
|CommandNotExist|10|Command does not exist|
|CommandMultiple|11|Multiple commands in the same request|
|CommandType|12|Command is known but it is the incorrect type (most commands are objects)|
|CommandSyntax|13|Command is known but contains invalid syntax. `m` will contain the offending parameter if known.|


## Keys
|Name|Value|Meaning
|:---|:---:|:---|
|KeySet|20|Key value is set.|
|KeyUpdated|21|Key already existed and has been updated|
|KeyNotExist|22|Key does not exist|
|KeyExist|23|Key exist (i.e. with `KV_CONTAINS`)|
|KeyRemoved|24|Key deleted/removed|
|KeyMissing|26|Key param not in command, i.e. `keys`|
|KeyTypeInvalid|27|Key wrong type. Must always be a string|


## Values
|Name|Value|Meaning
|:---|:---:|:---|
|ValueMissing|40|Expected value not present|
|ValueTypeInvalid|41|Value has incorrect type|
|ValueSize|42|Value exceeds maximum size (NOTE: if the payload size exceeds the maximum, this is not returned)|


## Find
These all relate to the `KV_FIND` command.

|Name|Value|Meaning
|:---|:---:|:---|
|FindNoPath|60|No `path`|
|FindNoOperator|61|No operator|
|FindInvalidOperator|62|Operator not permitted|



## Session
|Name|Value|Meaning
|:---|:---:|:---|
|SessionNotExist|100|Session does not exist (token is incorrect)|
|SessionTokenInvalid|101|Token is invalid (i.e. not a string)|

