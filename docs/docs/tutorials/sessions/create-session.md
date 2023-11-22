---
sidebar_position: 2
---

# Create Session

A session is created with the `SH_NEW` command.

The only required parameter is a `name`:

```json title="Session Never Expires"
{
  "SH_NEW":
  {
    "name":"session1"
  }
}
```

Expiry settings aren't set, so by default this session never expires.

The server responds with a `SH_NEW_RSP` containing the `name` and the `tkn` (token):

```json title="New Session Response"
{
  "SH_NEW_RSP":
  {
    "name":"session1",
    "tkn":8467384731478681233
  }
}
```

The token is used in subsequent `KV_` commands to access its data or with `SH_END` to end the session.


<br/>

## Expiring Session
Session expiry is useful for controling when data is deleted, releasing memory.

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
We create a session that expires after 10 minutes, but the session is not deleted, only its data:

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
After this session expires, its token remains valid for further use, but its data is deleted.

:::note
To clear a session's data it does not need to expire, you can use `KV_CLEAR` any time.
:::