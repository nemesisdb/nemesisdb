---
sidebar_position: 7
---

# SH_END_ALL
Ends all sessions, deleting all sessions and their data.

No parameters.

```json
{
  "SH_END_ALL": {}
}
```

## Response

`SH_END_ALL_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|cnt|unsigned int|Number of sessions deleted|

<br/>

Possible status values:

- Ok
