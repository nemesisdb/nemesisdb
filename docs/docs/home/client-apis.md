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
- The client must support 64-bit integers (required by the session token)


## Sessions

- A client can create unlimited sessions (within memory limitations)
- A session doesn't belong to a particular client/user - if a client has a valid session token, it can access the data
- A session ends with `SH_END` or when it expires if created with a duration
- When a session expires, there is no notification
- Session commands always respond (`SH_SAVE` can send two responses)

## KV

- All KV commands require a session token
- Session tokens must be handled as 64-bit unsigned integers
- `KV_SETQ` and `KV_ADDQ` only respond on error, all other commands always respond


## Basic Lifecycle

- Open the WebSocket connection
- Create session: `SH_NEW`
- Access session data: `KV_SET`, `KV_GET`, `KV_FIND`, etc
- End session: `SH_END` or allow to expire (if created to do so)
