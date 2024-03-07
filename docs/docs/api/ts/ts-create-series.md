---
sidebar_position: 1
---

# TS_CREATE
Creates a time series.


|Param|Type|Description|
|:---|:---:|:---|
|name|string|Name for the series. Must not be empty.|
|type|string|Must be "Ordered"|

The `name` is used in subsequent `TS_` commands.

## Response

`TS_CREATE_RSP`

See the [response status](./../TS-Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|name|string|The series name used in the original request|


Possible status values:

- Ok

