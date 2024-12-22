---
slug: /
displayed_sidebar: homeSidebar
---

# NemesisDB

NemesisDB is an in-memory key-value database with support arrays.

- The query interface is JSON over a WebSocket
- Keys can be persisted to the filesystem
- Persisted data can be loaded at startup or runtime
- Arrays:
  - Unsorted for JSON objects, strings and integers
  - Sorted for strings and integers
- Lists:
  - Node based container for JSON objects


## Arrays
- An array is a fixed size container, using contigious memory
- Unsorted arrays for strings, integers and JSON objects
- Sorted arrays for integers and strings
- Sorted arrays allow for intersection (more operations will be added later)

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

