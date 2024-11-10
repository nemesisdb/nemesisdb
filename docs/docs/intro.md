---
slug: /
displayed_sidebar: homeSidebar
---

# NemesisDB

NemesisDB is an in-memory JSON key-value database.

- The query interface JSON over a WebSocket
- Key Value can be with or without sessions
- Session and key value data can be persisted to the filesystem, then loaded on startup or at runtime
  
The database supports sessions which store keys in a dedicated map per session.

## Sessions Disabled
- A single map contains all keys
- All keys must have a unique name to avoid overwriting an existing key  
- Lower memory usage and latency
- No key expiry support

## Sessions Enabled
- Each session has a dedicated map
- A session can expire, after which, the keys are deleted
- A session is identified by a session token (an unsigned int)
- The session token is used in commands to interact with data in that session


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
- [First Steps: KV without sessions](./tutorials/first-steps-kv/setup)
- [First Steps: KV with sessions](./tutorials/first-steps/setup)

