---
sidebar_position: 3
---

# End Session
A session ends when it expires or when commanded by `SH_END`.

- When commanded by `SH_END` the session and its data are deleted
- When a session expires, there is a flag which controls if the session is deleted or just its data


By deleting the data you are releasing server memory for other sessions.


```json title="End Session"
{
  "SH_END":
  {
    "tkn":"8467384731478681233"
  }
}
```

The server confirms:

```json title="End Session"
{
  "SH_END_RSP":
  {
    "st":1,
    "tkn":"8467384731478681233"
  }
}
```

`st` is the status, `1` means "Ok".
