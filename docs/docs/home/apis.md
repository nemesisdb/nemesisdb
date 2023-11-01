---
sidebar_position: 20
displayed_sidebar: homeSidebar
---

# APIs

NemesisDB is new so there are no known client APIs yet. 

If you have an API or are plan to do so, feel free to contact via [Twitter](https://twitter.com/nmsisdb) for help or command request.


## Clients

- The query interface is a WebSocket
- It is not encrypted (HTTP/WS rather than HTTPS/WSS) currently

## Sessions

- A session is required to use the KV commands
- A client can create many sessions
- A session doesn't belong to a particular client - if a client has a valid session token, they can access the data
- You can call `SH_END` on a session even if it was created with expiry settings


## KV

- All KV commands require a session token
- Session tokens are strings (max/min lengths haven't been defined yet)
- `KV_SETQ` and `KV_ADDQ` only respond on error, all other commands always respond


## Typical Lifecycle

- Open the WebSocket connection
- Create sessions: `SH_NEW`
- Store data: `KV_SET`/`KV_SETQ` or `KV_ADD`/`KV_ADDQ`
- Get data: `KV_GET`
- Session info: `SH_INFO`
- End sessions: `SH_END`

