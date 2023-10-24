---
sidebar_position: 1
---

# SH_NEW
Creates a new session.

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|A session name. May be used when creating session token for shared sessions|Y|
|shared|bool|Default: `false`<br/> `true`: the session token is created so that the token can be retrieved by the token name with `SH_OPEN`. See [below](#shared-sessions)|N|
|expiry|object|Default: never expires <br/>Defines session expiry settings. See below.|N|

<br/>

`expiry`:

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|duration| unsigned int|Time in seconds until the session expires |Y|
|deleteSession| bool|Flag indicating if the session should be deleted. If `false`, only the data is deleted|Y|


<br/>


## Response

`SH_NEW_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|name|string|Session name as used in the request.|
|tkn|string|Session token|
|st|unsigned int|Status|

Possible status are:

- Ok
- CommandSyntax
- ValueMissing (no `name`)
- ValueTypeInvalid (`name` is wrong type)
- ValueSize (`name` is empty)

<br/>

## Examples

### Never expires, not shared

```json
{
  "SH_NEW": {
    "name": "user1000"
  }
}
```

### Expires after 1 minute, deletes session on expire, not shared

```json
{
  "SH_NEW": {
    "name": "sesh1",
    "expiry": {
      "duration": 60,
      "deleteSession":true
    }
  }
}
```

### Expires after 1 minute, only deletes data on expire, not shared

```json
{
  "SH_NEW": {
    "name": "sesh1",
    "expiry": {
      "duration": 60,
      "deleteSession":false
    }
  }
}
```

### Shared session, never expires

```json
{
  "SH_NEW": {
    "name": "sesh1",
    "shared": true
  }
}
```

<br/>


## Shared Sessions
Shared sessions don't refer to authentication - a session's data can be accessed by any client using the session token, there's no session authentication.

But that means clients knowing the session token. This may not be possible or practical because all services using the shared session must have the token.

A shared service helps by using the `name` to generate the session tkn. Other clients then use `SH_OPEN` with the same session name and receive the same session token.

