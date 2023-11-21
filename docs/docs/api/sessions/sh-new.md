---
sidebar_position: 1
---

# SH_NEW
Creates a new session.

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|The session name.<br/> If the session is shared, the name can be used in `SH_OPEN` to get the session token. |Y|
|shared|bool|Default: `false`<br/> `true`: the session token is created so that the token can be retrieved by the token name with `SH_OPEN`. See [below](#shared-sessions)|N|
|expiry|object|Default: never expires <br/>Defines session expiry settings. See below.|N|

<br/>

`expiry`:

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|duration| unsigned int|Time in seconds until the session expires |Y|
|deleteSession| bool|Flag indicating if the session should be deleted. If `false`, only the data is deleted|Y|


<br/>

## Session Name
For a session that is not shared (`"shared":false`) the name has no meaning to the server other than it can't be empty. The name is returned in the response so clients can match the name to a session token when working asynchronously.

If the session is shared the name can be used in [`SH_OPEN`](./sh-open.md) to get the session token from its name. This allows services/apps to access the same session without having to exchange the token.


## Response

`SH_NEW_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|name|string|Session name as used in the request.|
|tkn|uint|Session token|
|st|unsigned int|Status|

Possible status values:

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

