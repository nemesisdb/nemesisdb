---
sidebar_position: 1
displayed_sidebar: clientApisSidebar
sidebar_label: server_info
---

# server_info
Returns minimal server information.


## Returns
`dict`

|Key|Type|Meaning|
|---|---|---|
|persistEnabled|bool|Indicates if persistence is enabled|
|serverVersion|string|The server version in format `major.minor[.revision]`. If `revision` is 0, it is not included.|



## Examples

```py
client = KvClient()
await client.open('ws://127.0.0.1:1987/')

info = await client.server_info()
print(info)
```

```py
client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

info = await client.server_info()
print(info)
```