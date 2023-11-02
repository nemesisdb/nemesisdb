---
sidebar_position: 20
displayed_sidebar: homeSidebar
---

# Client APIs

NemesisDB is new so there are no known client APIs yet. 

If you have an API or plan to do so, feel free to contact via [Twitter](https://twitter.com/nmsisdb) for questions, feedback or requests.


## Clients

- The query interface is a WebSocket
- It is not encrypted (HTTP/WS rather than HTTPS/WSS) currently


## Sessions

- A client can create many sessions
- A session doesn't belong to a particular client - if a client has a valid session token, they can access the data


## KV

- All KV commands require a session token
- Session tokens are strings (max/min lengths haven't been defined yet)
- `KV_SETQ` and `KV_ADDQ` only respond on error, all other commands always respond


## Basic Lifecycle

- Open the WebSocket connection
- Create session(s): `SH_NEW`
- Access session data, i.e.:
    - Store: `KV_SET`/`KV_SETQ` or `KV_ADD`/`KV_ADDQ`
    - Get: `KV_GET`
    - etc
- End session(s): `SH_END`

