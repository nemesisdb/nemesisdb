---
sidebar_position: 1
displayed_sidebar: homeSidebar
---

# TLDR

NemesisDB is a JSON in-memory database:

- Data is stored in memory and is not saved to disk unless commanded
- Data is stored in sessions
- A session groups related data (similar to hashes in Redis)
- You can create as many sessions as required (within memory limitations)


## Sessions

- A session is just a data container. They are called sessions because data is intended to be stored for a period of time before being deleted
- A session ends when either:
  1. It is commanded
  2. It expires (a session can live forever or have a duration set)
- When a session is commanded to end, the session and its data are deleted
- When a session expires, its data is always deleted but you can choose if you also want to delete the session


### Examples
Examples of a session are:
- Music streaming app: each user has a session for storing their playlist
- One Time Password: create a session for each OTP, which automically expires


### Advantages

- Sessions offer control of server memory
- When searching data, only the session's data is searched, rather than the whole database
- Operations on a session does not affect another session's data
- Grouping data in sessions means keys only need to be unique within each session


:::note
Your database can have just one session that never expires but then you lose the benefits of grouping data and control of server memory.
:::
