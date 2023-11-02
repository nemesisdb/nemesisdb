---
sidebar_position: 1
displayed_sidebar: homeSidebar
---

# TLDR

NemesisDB is a JSON in-memory database:

- Data is stored in memory and is not saved to disk unless commanded
- Data is stored in sessions: a session groups related data (similar to hashes in Redis)

:::info
At the moment data cannot be saved to disk, but a future version will have a save command
:::


## Sessions

- A session is just a data container. They are called sessions because data is intended to be stored for a period of time before being deleted
- Examples of a session are:
  - Whilst a user is logged into your app
  - Whilst you're waiting for a user to confirm a One Time Password
  - Whilst you're waiting for confirmation the data has been stored in the primary database
- A session ends when either:
  1. It is commanded
  2. It expires (a session can live forever or have a duration set)
- When a session is commanded to end, the session and its data are always deleted
- When a session expires, its data is always deleted but you can choose if you also want to delete the session


### Advantages

- Sessions offer control of server memory
- You can use a session as you would a class in a programming language: it represents related data (such as a user, a package out for delivery, app settings, etc)
- Grouping data in sessions means keys only need to be unique within each session
- The engine is modern C++, designed to take advantage of multicore CPUs. At the moment most effort is on features rather than performance but this will
change as functionality stabilises


:::note
Your database can have just one session that never expires, but then you lose the benefits of grouping data and easier control of deleting data.
:::
