---
sidebar_position: 2
---

# SH_NEW
Creates a new session.

The response includes a session token which is a 64-bit unsigned integer. The session token is used in the `KV_` commands to set, update, find, etc session data.

To create a session that never expires:

```json
{
  "SH_NEW":{}
}
```

which produces a response:

```json title="The name used in the request is included in the response with the token"
{
  "SH_NEW_RSP":
  {
    "st": 1,
    "tkn": 260071646955705531
  }
}
```

<br/>

The session token (`tkn`) is used in subsequent `KV_` commands, for example:

```json
{
  "KV_SET":
  {
    "tkn":260071646955705531,
    "keys":
    {
      "forename":"Billy",
      "surname":"Bob"
    }
  }
}
```
<br/>

A session can have optional `expiry` settings:

- a session can be given a duration, which when reached, the session's keys are deleted
- the session can also be deleted

<br/>

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|expiry|object|Default: never expires <br/>Defines session expiry settings. See below.|N|

<br/>
<br/>

`expiry`:

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|duration| unsigned int|Time in seconds until the session expires |Y|
|deleteSession| bool|Flag indicating if the session should be deleted. If `false`, only the data is deleted|Y|

<br/>

## Session Expiry
A session's expiry time is extended by its `duration` when it is accessed by any `KV_` command - if a session is not accessed for `duration` seconds, it will expire. 

<!-- 
## Session Name
For a session that is not shared (`"shared":false`) the name has no meaning to the server other than it can't be empty. The name is returned in the response so clients can match the name to a session token when working asynchronously.

If the session is shared the name can be used in [`SH_OPEN`](./sh-open.md) to get the session token from its name. This allows services/apps to access the same session without having to exchange the token.

<br/> -->



<!-- 
<br/>
## Shared Sessions
A session can be accessed by any client using the session token but it may be cumbersome to distribute a session token between clients.

A shared session helps by using the `name` to generate the session token. Other clients use `SH_OPEN` with the same name and receive the same session token.

:::note
This only applies when a session is shared. If a session is not shared the `name` does not take part in token generation.
::: -->

<br/>

## Response

`SH_NEW_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|unsigned int|Session token|
|st|unsigned int|Status|

Possible status values:

- Ok
- CommandSyntax
- ValueMissing (no `name`)
- ValueTypeInvalid (`name` is wrong type)
- ValueSize (`name` is empty)

<br/>

## Examples

```json title='Never expires'
{
  "SH_NEW":{}
}
```
<br/>

```json title='Expires after 1 minute, session not deleted'
{
  "SH_NEW":
  {
    "expiry":
    {
      "duration": 60,
      "deleteSession":false
    }
  }
}
```

<br/>

```json title='Expires after 1 minute, session deleted'
{
  "SH_NEW":
  {
    "expiry":
    {
      "duration": 60,
      "deleteSession":true
    }
  }
}
```
