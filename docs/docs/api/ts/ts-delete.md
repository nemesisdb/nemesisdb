---
sidebar_position: 2
---

# TS_DELETE
Delete a time series and its index maps.


|Param|Type|Description|
|:---|:---:|:---|
|name|string|Name of the series to delete|


## Response

`TS_DELETE_RSP`

See the [response status](./../TS-Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|name|string|The series name used in the original request|


Possible status values:

- Ok
- SeriesNotExist

