---
sidebar_position: 1
---

# SH_NEW
Creates a new session.

If the session is created without error, the response includes a session token which a 64-bit unsigned integer. The session token is then used in the `KV_` commands to set, update, find, etc session data.

A session requires at least a name but can have optional settings:

- `expiry` : a session can be given a duration, which when reached, the session data is deleted. The session can also be deleted
- `shared` : this allows clients to get a session token from the session name. This is useful if the same session data is required across separate services without having to distribute the token

There is no session authentication - if a client has the token it can access the data.

<br/>

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

<br/>

## Session Expiry
A session's expiry time is extended by its `duration` when it is accessed by any `KV_` command. In other words, if a session is not accessed for `duration` seconds, it will expire. By accessing the session's data you are extending the expiry because it suggests that session is required.

<br/>

## Shared Sessions
A session can be accessed by any client using the session token but it may be difficult to distribute a session token between clients.

A shared session helps by using the `name` to generate the session token. Other clients use `SH_OPEN` with the same name and receive the same session token.

:::note
This only applies when a session is shared. If a session is not shared the `name` does not take part in token generation.
:::

<br/>

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
  "SH_NEW":
  {
    "name": "user1000"
  }
}
```

### Expires after 1 minute, deletes session on expire, not shared

```json
{
  "SH_NEW":
  {
    "name": "sesh1",
    "expiry":
    {
      "duration": 60,
      "deleteSession":true
    }
  }
}
```

### Expires after 1 minute, only deletes data on expire, not shared

```json
{
  "SH_NEW":
  {
    "name": "sesh1",
    "expiry":
    {
      "duration": 60,
      "deleteSession":false
    }
  }
}
```

### Shared session, never expires

```json
{
  "SH_NEW":
  {
    "name": "shared1",
    "shared": true
  }
}
```

Because this session is shared, a client can use `SH_OPEN` with the name to retrieve the session token:

```json
{
  "SH_OPEN":
  {
    "name":"shared1"
  }
}
```

This returns the same token as `SH_NEW`.

