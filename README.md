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

## Design
The design aims to separate I/O and session data threads:

- Core(s) are dedicated for I/O operations (WebSocket server)
- Remaining core(s) are dedicated to handle sessions

Each thread is assigned to a core.

The session threads receive commands via a boost::fiber channel and execute the command. This is a convenient way to serialise the execution of commands: because a session thread fully executes the command before popping the next, we don't need to worry about the usual multithreaded issues.

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

### Store String Key
```json
{
  "KV_SET":
  {
    "tkn":448892247316960382,
    "keys":
    {
      "user":"bob"
    }
  }
}
```

### Get Key
```json
{
  "KV_GET":
  {
    "tkn":448892247316960382,
    "keys": ["user"]
  }
}
```


<br/>

## Code
The server is the binary, defined in `server/server.cpp` and the remaining code in `core`. 

1. `server.cpp` checks the command line params, the config and then calls `core/Kv/KvServer::run()`
2. `KvServer::run()` runs the WebSocket server, creating reusable sockets
3. When a message arrives, the `.message` handler is called, which parses the JSON and if valid, calls `KVHandler::handle()`
4. `KvHandler` validates the command then submits it to a `KvPoolWorker`
5. The command is initially pushed to the pool worker's channel, it'll then be popped and executed

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

