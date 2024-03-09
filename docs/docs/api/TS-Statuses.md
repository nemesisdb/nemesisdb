---
sidebar_position: 3
displayed_sidebar: apiSidebar
---

# Time Series Response Status
The status `st` is an unsigned integer returned by all commands.

The API docs list friendly names which this table maps to integer values:


|Name|Value|Meaning
|:---|:---:|:---|
|Ok|1|Command successful, everything good|
|OpCodeInvalid|2|The "op code" of the WebSocket message must be text|
|JsonInvalid|3|JSON could not be parsed|
|UnknownError|4||
|CommandSyntax|5|General syntax error, such a request with no command: `{ }`|
|CommandNotExist|6|Command not recognised|
|ParamMissing|7|A missing parameter|
|ParamType|8|Type of param is invalid|
|ParamValue|9|A value is empty or above a maximum|
|RngSize|10|A time range (`rng`) or range operator (`[]`) not expected size|
|RngValues|11|Values in the range invalid, i.e. minimum is higher than maximum|
|SeriesNotExist|20|Series name does not exist|
|SeriesExists|21|Series name alreay exists|
|SeriesType|22|Series type not supported|
|IndexExists|40|A member is already indexed|
|NotIndexed|41|A member used in `where` is not indexed|


