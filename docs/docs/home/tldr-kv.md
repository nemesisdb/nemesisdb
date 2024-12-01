---
sidebar_position: 1
displayed_sidebar: homeSidebar
---

# TLDR - Key Value

NemesisDB is a JSON in-memory database:

- The server maps a JSON key to its value
- Keys can be stored inside or outside of a session


## Independent/Top Level Keys

- A single map stores all top level keys
- Keys must be named unique to avoid overwriting
- Lower latency


## Session

- A session groups related keys
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

