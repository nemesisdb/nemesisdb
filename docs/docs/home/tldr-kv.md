---
sidebar_position: 1
displayed_sidebar: homeSidebar
---

# TLDR - Key Value

NemesisDB is a JSON in-memory database:

- Key-value can be with or without sessions enabled
- Session and key value data can be saved to disk, then loaded at startup or at runtime with a command


## Sessions Disabled

- A single map stores all key-values
- Keys must be named uniquely over the entire database
- No session token required
- Lower memory usage and latency


## Sessions Enabled

- You can create as many sessions as required (within memory limitations)
- A session groups related data (similar to hash sets in Redis)
- A session is just a data container. They are called sessions because data is intended to be stored for a period of time before being deleted
- A session ends when either:
  1. It is commanded with `SH_END` or `SH_END_ALL`
  2. It expires
- When a session is commanded to end, the session and its data are deleted
- When a session expires, its data is always deleted but you can choose to delete the session



### Advantages

- Grouping data in sessions means keys only need to be unique within each session
- Sessions offer easier control of data in cases where each session stores data for a particular entity (user, results, device, etc)
- When access keys, only the session's map is accessed
- Operations on a session does not affect another session's keys

<br/>

:::note
You can have just one session that never expires, in which case you should consider disabling sessions.
:::
