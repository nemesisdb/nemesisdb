---
sidebar_position: 1
displayed_sidebar: clientApisSidebar
---

# sv_info
Returns minimal server information.


```py
sv_info() -> dict:
```

## Returns
|Key|Type|Meaning|
|---|---|---|
|persistEnabled|bool|Indicates if persistence is enabled|
|serverVersion|string|The server version|



## Raises
- `ResponseError` if query fails

