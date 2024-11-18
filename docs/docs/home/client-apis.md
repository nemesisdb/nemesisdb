---
sidebar_position: 20
displayed_sidebar: homeSidebar
---

# Client APIs

There is a Python API in the early stages of development.

If you have an API or plan to do so, feel free to contact via [Twitter](https://twitter.com/nmsisdb) for questions, feedback or requests.


## Clients

- The query interface is a WebSocket
- Only HTTP supported currently
- Client must support 64-bit integers for sessions



## Sessions Disabled
- A single map for all keys


## Sessions Enabled

- Each session has a dedicated map
- A session is uniquely identified by a session token (64bit unsigned int)
- A client can create unlimited sessions (within memory limitations)
- A session doesn't belong to a particular client/user - if a client has a valid session token, it can access the data
- A session can expire. When a session expires the keys are always deleted, but optionally the session can also be deleted
