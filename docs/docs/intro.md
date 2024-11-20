---
slug: /
displayed_sidebar: homeSidebar
---

# NemesisDB

NemesisDB is an in-memory JSON key-value database.

- The query interface JSON over a WebSocket
- Session support: each session has a dedicated map and can expire
- Keys can be persisted to the filesystem
- Persisted data can be loaded at startup or runtime
  
## Sessions Disabled
- A single map contains all keys
- Keys must have a unique name to avoid overwriting an existing key  
- Lower memory usage and latency
- No key expiry support

## Sessions Enabled
- Each session has a dedicated map
- A session can expire, after which, the keys are deleted
- A session is identified by a session token (an unsigned int)
- The session token is used in commands to access with keys in that session


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

