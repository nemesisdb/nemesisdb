# NemesisDB
NemesisDB is a session JSON database:

JSON:
- All data and commands are JSON
- Commands are submitted via a WebSocket

Session:
- Each session has a dedicated map
- A session can live forever, but the intent is for each session to have an expiry
- When a session expires its data is deleted, and optionally the session can be deleted

Examples of sessions:
- Whilst a user is logged into an app
- When a device comes online
- When an One Time Password is created
- Whilst a user is completing a multi-page online form

<br/>

## Sessions
The purpose of sessions are:
- Each session only contains data required for that session, rather than a single large map
- When accessing (get, set, etc) data, only the data for a particular session is accessed
- Controlling key expiry is simplified because it is sessions that expire, not individual keys

Rather having a single large map of keys, they are split into sessions.

For example, a system which manages 100K users, each of which requires 10 keys:

- We have 100K sessions, each containing 10 keys.
- Each session represents a user's data
- Managing expiring sessions is simpler because rather than monitoring 1M keys, only the 100K sessions are managed
- When accessing session data, only an individual session's map is accessed

More info [here](https://docs.nemesisdb.io/tutorials/sessions/what-is-a-session).

<br/>

## Install
NemesisDB is available as Debian package or Docker image:

- Package:  [Releases](https://github.com/nemesisdb/nemesisdb/releases) 
- Docker: [Docker Hub](https://hub.docker.com/r/nemesisdb/nemesisdb/tags)

You can also build from source, instructions below.


<br/>

## Design
The design aims to separate I/O and session data threads:

- Core(s) are dedicated for I/O operations (WebSocket server)
- Remaining core(s) are dedicated to handle sessions

Each thread is assigned to a core.

With 4 cores, 3 are assigned to I/O and 1 for session data:

![4 cores](https://20aac7f3a5b7ba27bcb45d6ccf5d4c71.cdn.bubble.io/f1707487526533x999369528600104800/nemesis_cores_4.png?_gl=1*b29xz5*_gcl_au*MTc4NTg0NDIyMy4xNzA3NDIwNzA2*_ga*MTcwMTY5ODQzNC4xNjk3NTQyODkw*_ga_BFPVR2DEE2*MTcwNzQ4Njc4MS4yNC4xLjE3MDc0ODc2MDIuNjAuMC4w)

<br/>

With 6 cores, 4 are assigned to I/O and 2 for session data:

![6 cores](https://20aac7f3a5b7ba27bcb45d6ccf5d4c71.cdn.bubble.io/f1707487522110x648951654930151300/nemesis_cores_6.png?_gl=1*j7jxla*_gcl_au*MTc4NTg0NDIyMy4xNzA3NDIwNzA2*_ga*MTcwMTY5ODQzNC4xNjk3NTQyODkw*_ga_BFPVR2DEE2*MTcwNzQ4Njc4MS4yNC4xLjE3MDc0ODc2MDIuNjAuMC4w)


If there are multiple session data threads, the data is distributed over the threads, all of which are independent (i.e. no data is shared between the session data threads). 

Command execution sequence:

I/O Thread: 
- Receive Command
- Parse JSON
- Validate Command
- Map session token to session worker thread
- Submit to session thread (typically async, but sync for some commands)
  - This pushes the command onto the thread's channel

Session Thread:
- Wait until a command arrives on the channel then pop it
- Execute command
- If async:
  - Send response on the same I/O thread that received the command
- If sync:
  - Call handler on session thread and send response on I/O thread

Commands for sessions that are handled by different session threads can be executed concurrently because they are handled by different threads.

> [!NOTE]
> There is a 4 thread max, so at most there will be 3 I/O threads and 1 session worker thread.
> - The limit is set by `NEMESIS_MAX_CORES` in `NemesisCommon.h`
> - The ratio of I/O to session pool threads is defined by `CoresToIoThreads` map in `KvServer::init()`. This defines how many threads are assigned to I/O, the remaining are for session pool(s).
>
> 99% of testing has been with limit as 4

<br/>

## Build - Linux Only
1. Clone via SSH with submodules: `git clone --recursive git@github.com:nemesisdb/nemesisdb.git`
2. Prepare and grab vcpkg libs: `cd nemesisdb && ./prepare_vcpkg.sh` (this takes a few minutes)
3. With VS Code (assuming you have C/C++ and CMake extensions):
    - VS Code: `code .`
    - Select kit (only tested with GCC 12.3.0)
    - Select Release variant
    - Select target as nemesisdb
    - Build
4. Binary is in `server/Release/bin`

<br/>

## Run
1. `cd server/Release/bin`
2. `./nemesisdb --config=../../configs/default.json`
    - Server WebSocket listening on `127.0.0.1:1987` (defined in `default.json`)
3. `ctrl+c` to exit

<br/>

## Commands
The quickest way to get started is to use software such as [Postman](https://www.postman.com/downloads/) to query the WebSocket

The [first steps](https://docs.nemesisdb.io/tutorials/first-steps/setup) guide is a good place to start.

### Session Info
```json
{
  "SH_INFO_ALL":{}
}
```

### Create Session
```json
{
  "SH_NEW":
  {
    "name":"session1"
  }
}
```
This returns the session token `tkn`, for example: `448892247316960382` .

### Store String Keys
Store keys `username`, `email`, `city` with string values and `age` with a number value:

```json
{
  "KV_SET":
  {
    "tkn":448892247316960382,
    "keys":
    {
      "username":"bob",
      "email":"bob@email.com",
      "city":"Paris",
      "age":45
    }
  }
}
```

### Store Object Key
Rather than having separate keys, store the values in a single object with key `profile`:

```json
{
  "KV_SET":
  {
    "tkn":448892247316960382,
    "keys":
    {
      "profile":
      {
        "username":"bob",
        "email":"bob@email.com",
        "age":45,
        "city":"Paris"
      }
    }
  }
}
```

### Get Keys
Get single key for a string:
```json
{
  "KV_GET":  {
    "tkn":448892247316960382,
    "keys": ["username"]
  }
}
```

Get multiple keys:
```json
{
  "KV_GET":  {
    "tkn":448892247316960382,
    "keys": ["username", "city", "age"]
  }
}
```

Get key with object value:
```json
{
  "KV_GET":  {
    "tkn":448892247316960382,
    "keys": ["profile"]
  }
}
```

<br/>

## Code
The server is the binary in `server/server.cpp` and the remaining code in `core`.

1. `server.cpp` checks the command line params, the config and then calls `core/Kv/KvServer::run()`
2. `KvServer::run()` runs the WebSocket server, creating reusable sockets
3. When a message arrives, the `.message` handler is called, which parses the JSON. If valid, it calls `KvHandler::handle()`
4. `KvHandler` validates the command then submits it to a `KvPoolWorker`
5. The session token is used to determine the pool worker responsible for its session data
6. The command is pushed to pool worker's channel, which pops and executes the command

<br/>

## External Libraries
Externals are either GitHub submodules or managed by [vcpkg](https://vcpkg.io/en/).

Server:
- uWebsockets : WebSocket server
- jsoncons : json
- plog : logging
- Boost Fiber : channels (multiple producer/multiple consumer container)
- Boost Program Options : argv options
- ankerl : unordered_dense::segmented map
- uuid_v4 : create UUIDs with SIMD

Tests:
- nlohmann json
- Boost Beast
- Google test

<br/>

## License
See LICENCE file:
 - Use in commercial software but credit must be given
 - No liability
 - No use of trademark

<br/>

## Project Status
The software is still alpha state with upcoming improvements. 

The code has been open sourced to gain users and contributors:

### Source
Code improvemements are always possible and of course bug fixes.

### Client APIs
The only client API is the test API in this repo, which is only for testing. Client APIs in more common languages are welcome, particularly JS, C# and Python.

