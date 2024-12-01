---
sidebar_position: 2
---

# SH_NEW
Creates a new session.

Each session has a dedicate map so changes to a session's keys does not affect another session.

A session exist forever or expire (see [options](#session-expiry)).


The response includes a session token which is a 64-bit unsigned integer. The session token is used in the `SH_` commands to set, update, find, etc session data.

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

The session token (`tkn`) is used in subsequent `SH_` commands, for example:

```json
{
  "SH_SET":
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

## Session Expiry

A session can have `expiry` settings:

- a session can be given a duration, which when reached, the session's keys are deleted
- the session can also be deleted
- the session's expire time can be extend on each call to `set/add` or `get`

<br/>

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|expiry|object|Default: if not present, session never expires <br/>Defines session expiry settings. See below.|N|

<br/>
<br/>

`expiry`:

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|duration| unsigned int|Time in seconds until the session expires |Y|
|deleteSession| bool|`true`: session is deleted when it expires<br/>`false`: only the keys are deleted (default)|N|
|extendOnSetAdd|bool|`true`: on each set or add, the expire time is extended by `duration`<br/>`false`: default|N|
|extendOnGet|bool|`true`: on each get, the expire time is extended by `duration`<br/>`false`: default|N|

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


<br/>

## Examples

```json title='Never expires'
{
  "SH_NEW":{}
}
```
<br/>

```json title='Expires every 60 seconds, session not deleted'
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

<br/>

```json title='Expires every after 60 seconds, each call to get extends by 60 seconds'
{
  "SH_NEW":
  {
    "expiry":
    {
      "duration": 60,
      "extendOnGet":true
    }
  }
}
```