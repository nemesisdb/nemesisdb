---
sidebar_position: 1
displayed_sidebar: clientApisSidebar
---

# Overview


## Python
:::important
This API is new and likely to change. 
:::

- `from ndb.client import NdbClient`
- The `NdbClient` class contains:
  - `open()` and `close()` for the connection
  - Commands are sent with functions beginning:
    - `kv_` for keys not in a session, i.e. `kv_set()`
  - If a command returns an failure, an `ndb.ResponseError` is raised
    - The exception message contains the error code
    - The exception contains `rsp` which is the full response