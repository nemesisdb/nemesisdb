---
sidebar_position: 2
---

# Create Session

A session is created with the `SH_NEW` command.

The minimum required is a `name`:

```json title="Session Never Expires"
{
  "SH_NEW":
  {
    "name":"session1"
  }
}
```

We haven't set expiry properties so this session never expires.

The server responds with a `SH_NEW_RSP` containing the `name` and the token, `tkn`:

```json title="New Session Response"
{
  "SH_NEW_RSP":
  {
    "name":"session1",
    "tkn":"8467384731478681233"
  }
}
```



:::tip
The session token is a 64-bit integer as a string. This may change in the future so always treat the token as a string.
:::

<br/>

## Expiring Session
We can create a session that expires after a minute by setting duration:

```json title="Session Expires in 1 minute, deletes session"
{
  "SH_NEW":
  {
    "name":"session2",
    "expiry":
    {
      "duration": 60,
      "deleteSession":true
    }
  }
}
```

`deleteSession:true` means the session is also deleted.

<br/>

## Expiring Session, Delete Data Only
We can create a session that expires after 10 minutes but the session is not deleted, only its data:

```json title="Session Expires in 10 minute, deletes data only"
{
  "SH_NEW":
  {
    "name":"session3",
    "expiry":
    {
      "duration": 600,
      "deleteSession":false
    }
  }
}
```
