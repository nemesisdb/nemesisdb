---
slug: /
displayed_sidebar: homeSidebar
---

# NemesisDB

NemesisDB is an in-memory JSON key-value and timeseries database.

- The query interface is over a WebSocket using JSON
- Key Value can be with or without sessions
- Time series is basic but will be extended in future versions
  
Sessions Enabled:
  - Sessions are similar to Redis' hash sets: they group related data
  - Each sessoion has a dedicated map
  - A session can expire, after which, the keys are deleted
  - Sessions are created with the `SH_NEW` command, which returns a session token
  - The session token is used in key value commands to interact with data only in that session
  - Session data can be persisted to the filesystem
  - Session data can be restored from file at startup or with a command at runtime

Sessions Disabled:
  - A single map contains all key values  
  - No need to supply a session token with each command
  - Data can be persisted to the filesystem
  - Data can be restored from file at startup or with a command at runtime
  - Lower memory usage and latency

Time Series:
  - Basic support to store, get, index and conditionally find
  - No data persistence


<br/>

NemesisDB is available as an x86 64-bit Debian package, Docker image or can be built for Linux from [source](https://github.com/nemesisdb/nemesisdb).

<br/>

## Install

The install includes a default config to start the server on `127.0.0.1:1987`. This can be changed in the [config](./home/config) file. 

### Linux
- [Debian package](./home/install/package)
- [Docker](./home/install/docker/linux)

### Windows
- [Docker](./home/install/docker/windows)


<br/>

# Next Steps

- [Key Value TLDR](./home/tldr-kv)
- [Time Series TLDR](./home/tldr-ts)
- [First Steps](/tutorials/first-steps/setup)

