# NemesisDB
NemesisDB is an in-memory keyvalue JSON database:

- All data and commands are JSON over WebSockets
- The query interface is a WebSocket
- Keys can be saved to file
- Keys can be loaded at startup or runtime


API and further information in the [docs](https://docs.nemesisdb.io/).

Contents:
  - [Python API](#python-api)
  - [Design](#design)
  - [Install](#install)
  - [Build for Linux](#build---linux-only)
  

<br/>
<br/>



# Overview

To store a key the `KV_SET` command is used:

```json
{
  "KV_SET":
  {
    "keys": {"username":"desire"}
  }
}
```

Or we can store multiple keys:

```json
{
  "KV_SET":
  {
    "keys":
    {
      "forename":"James",
      "age":35,
      "address":
      {
        "city":"Paris"
      }
    }
  }
}
```

This sets three keys with value types:

|Key|Value Type|
|---|---|
|forename|string|
|age|integer|
|address|object|

<br/>

Use `KV_GET` to get keys:

```json
{
  "KV_GET":
  {
    "keys":["username"]
  }
}
```

This responds with:

```json
{
  "KV_GET_RSP":
  {
    "st": 1,
    "keys":
    {
      "username": "desire"
    }
  }
}
```
<br/>

|||
|---|---|
|KV_GET_RSP|The command name. A response to a command is always the command name with `_RSP` appended|
|st|Status code (`1` is success)|
|keys|The keys and values retrieved. If a key does not exist, it is omitted from the response.|


You can request multiple keys with `KV_GET`:

```json
{
  "KV_GET":
  {
    "keys":["forename", "age", "address"]
  }
}
```

With response:

```json
{
  "KV_GET_RSP": {
    "st": 1,
    "keys":
    {
      "forename": "James",
      "age": 35,
      "address":
      {
        "city": "Paris"
      }
    }
  }
}
```

<br/>
<br/>

# Sessions
Sessions can be disabled or enabled. Each session has a dedicated map for its keys and the session can exist forever or expire:

- Each session only contains data stored in that session
- When accessing (get, set, etc) data, only the data for a particular session is accessed
- With sessions disabled, keys cannot expire, they must be deleted manually
- With sessions enabled, a session can expire, deleting all keys in the session

You can create as many sessions as required (within memory limitations). When a session is created, a session token is returned (a 64-bit unsigned integer), so switching between sessions only requires using the appropriate token.

More info [here](https://docs.nemesisdb.io/tutorials/sessions/what-is-a-session).

## Sessions Disabled

- There is one map for all keys
- Keys cannot expire, they must be deleted by command
- Data is not segregated so keys must be unique over the entire database
- Do not need to create sessions to store keys
- Lower memory usage and latency

## Sessions Enabled
Rather than one large map, keys are stored per session:

- Each session has a dedicated map
- A session can live forever or expire after a defined duration
- When a session expires:
  - The keys are deleted
  - Optionally the session can be deleted

Examples of sessions:

- Each user that logs into an app
- Each connected device in monitoring software
- When an One Time Password is created
- Whilst a user is completing a multi-page online form

<br/>
<br/>


# Python API
There is an early version of a [Python API](https://github.com/nemesisdb/nemesisdb/tree/main/apis/python) with docs [here](https://docs.nemesisdb.io/client_apis/python/Overview).

Set then retrieve two keys, `username` and `password`:

```py
from ndb.kvclient import KvClient

client = KvClient()
await client.open('ws://127.0.0.1:1987/')

setSuccess = await client.set({'username':'billy', 'password':'billy_passy'})

if setSuccess:
  (getOk, values) = await client.get(('username','password'))
  print (values if getOk else 'Query failed')
```


<br/>
<br/>

# Install
NemesisDB is available as a Debian package and Docker image:

- Package:  [Releases](https://github.com/nemesisdb/nemesisdb/releases) 
- Docker: [Docker Hub](https://hub.docker.com/r/nemesisdb/nemesisdb/tags)

You can compile for Linux, instructions below.


<br/>
<br/>

# Design

As of version 0.5, the engine is single threaded. The instance is assigned to core 0 by default but can be configured in the server [config](https://docs.nemesisdb.io/home/config).

The multithreaded version is collecting GitHub dust on the [0.4.1](https://github.com/nemesisdb/nemesisdb/tree/0.4.1) branch.

<br/>

## Save and Restore
Session and key value data be saved to file and restored:

Sessions enabled:
- `SH_SAVE` save all sessions or particular sessions
- `SH_LOAD` to load data from file at runtime
- Use `--loadName` at the command line to load during start up

Sessions disabled:
- `KV_SAVE` to save all key
- `KV_LOAD` to load keys from file at runtime
- Use `--loadName` at the command line to load during start up


<br/>
<br/>


# Performance
Performance testing has not been a priority, but early indications suggest 90k-120k queries/second. This is in Azure on a single EPYC core setting individual keys from a C++ client. The query rate depends on key/value length, whether using `KV_SET` or `KV_SETQ` (only returns a response on an error) and network/CPU load. 

<br/>

# Build - Linux Only

> [!IMPORTANT]
> C++20 required.

1. Clone via SSH with submodules: `git clone --recursive git@github.com:nemesisdb/nemesisdb.git`
2. Prepare and grab vcpkg libs: `cd nemesisdb && ./prepare_vcpkg.sh`
3. With VS Code (assuming you have C/C++ and CMake extensions):
    - `code .`
    - Select kit (tested with GCC 12.3 and GCC 13.1)
    - Select Release variant
    - Select target as nemesisdb
    - Build
4. Binary is in `server/Release/bin`

<br/>

## Run
Start listening on `127.0.0.1:1987` with sessions disabled (default in `default.jsonc`):

`./nemesisdb --config=default.jsonc`

Use `ctrl+c` to exit.


<br/>
<br/>

# External Libraries
Externals are either GitHub submodules or managed by [vcpkg](https://vcpkg.io/en/).

Server:
- uWebsockets : WebSocket server
- jsoncons : json
- ankerl::unordered_dense for map and segmented map
- plog : logging
- uuid_v4 : create UUIDs with SIMD
- Boost Program Options : argv options

Tests:
- nlohmann json
- Boost Beast (WebSocket client)
- Google test

