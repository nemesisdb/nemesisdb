---
sidebar_position: 5
---

# SH_END
Ends a session, deleting the session data.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|unsigned int|Session token|Y|


<br/>

:::info
`SH_END` ignores expiry settings so a session created with expiry settings is ended immediately.
:::

## Response

`SH_END_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|unsigned int|Session token|
|st|unsigned int|Status|


Possible status values:

- Ok
- SessionNotExist
- SessionTokenInvalid 
