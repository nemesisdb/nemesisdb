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
- No session token requires
- Keys are not segregated:
  - i.e. if you're caching user data, all user data is stored in a single map, but with sessions you can create a separate sessions per user
  - Keys have to be unique across the entire database. With sessions, keys are only unique within a session because each session has a dedicated map
- Lower memory usage and latency

<br/>

## Sessions Enabled

- You can create as many sessions as required (within memory limitations)
- A session groups related data (similar to hash sets in Redis)
- A session is just a data container. They are called sessions because data is intended to be stored for a period of time before being deleted
- A session ends when either:
  1. It is commanded
  2. It expires (a session can live forever or have a duration set)
- When a session is commanded to end, the session and its data are deleted
- When a session expires, its data is always deleted but you can choose if you also want to delete the session



### Advantages

- Sessions offer easier control of data. In the example above when caching user data: when you want to delete data for a particular user, you just delete the user's session
- When searching data, only the session's map is searched
- Operations on a session does not affect another session's data
- Grouping data in sessions means keys only need to be unique within each session


:::note
You can have just one session that never expires, in which case you should consider disabling sessions.
:::
