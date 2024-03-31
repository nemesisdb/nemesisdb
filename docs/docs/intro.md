---
slug: /
displayed_sidebar: homeSidebar
---

# NemesisDB

NemesisDB is an in-memory JSON key-value and timeseries database.

- The query interface is over a WebSocket using JSON
- Key Value can be with or without sessions
- Session and key value data can be persisted to the filesystem, then loaded on startup or at runtime with a command
- Time series is basic but will be extended in future versions (no data persistence yet)
  
## Key Value

### Sessions Enabled
  - Sessions are similar to Redis' hash sets: they group related data
  - Each session has a dedicated map
  - A session can expire, after which, the keys are deleted
  - Sessions are created with the `SH_NEW` command, which returns a session token
  - The session token is used in key value commands to interact with data only in that session
  - Session data can be persisted to the filesystem
  - Session data can be restored from file at startup or with a command at runtime

### Sessions Disabled
  - A single map contains all key values  
  - Don't need to supply a session token with each command
  - Lower memory usage and latency

## Time Series
  - Basic support to store, get, index and conditionally find
  - No data persistence

<br/>

## Install

NemesisDB is available as an x86 64-bit Debian package, Docker image or can be built for Linux from [source](https://github.com/nemesisdb/nemesisdb).

The install includes a default config to start the server on `127.0.0.1:1987`. This can be changed in the [config](./home/config) file. 

### Linux
- [Debian package](./home/install/package)
- [Docker](./home/install/docker/linux)

### Windows
- [Docker](./home/install/docker/windows)


<br/>

## Next Steps

- [Key Value TLDR](./home/tldr-kv)
- [Time Series TLDR](./home/tldr-ts)
- [First Steps: KV without sessions](./tutorials/first-steps-kv/setup)
- [First Steps: KV with sessions](./tutorials/first-steps/setup)

