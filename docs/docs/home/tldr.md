---
sidebar_position: 1
displayed_sidebar: homeSidebar
---

# TLDR

NemesisDB is a JSON in-memory database:

- Data is stored in memory and is not saved to disk unless commanded
- At the moment data cannot be saved to disk, but a future version will have a save command
- Data is stored in sessions, a bit like hashes in Redis, in that a session groups related data


## Sessions

- A session is just a data container. They are called sessions because data is intended to be stored for a period of time before being deleted
- A session can have an expiry set. When a session expires, all its data is deleted
- A session can be:
  - Whilst a user is logged into your app
  - Whilst you're waiting for a user to confirm a One Time Password
  - Until the session data has been stored in the primary database


### Advantages

- Sessions offer control of server memory
- You can use a session as you would a class in a programming language: it represents related data (such as a user, a package out for delivery, app settings, etc)
- The engine is modern C++, designed to take advantage of multicore CPUs. At the moment most effort is on features rather than performance but this will
change as functionality stabilises


:::note
You can create one session that never expires, but then you lose the benefits of grouping data and easier control of deleting data.
:::
