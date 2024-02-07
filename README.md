# NemesisDB
NemesisDB is a session JSON database:

- All data and commands are JSON
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

Rather having a single large map of keys, it would be split into sessions.

For example, a system which manages 100K users, each of which requires 100 keys, we'd have 100K sessions, each having 100 keys:

- Managing expiring sessions is simpler because rather than monitoring 1M keys, only the 100K sessions are managed
- When accessing session data, only an individual session's map is accessed

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
 

<br/>

## External Libraries
Externals are either GitHub submodules or managed by [vcpkg](https://vcpkg.io/en/).

Server:
- uWebsockets : WebSocket server
- plog : logging
- jsoncons : json
- Boost Fiber : channels (multiple producer/multiple consumer container)
- Boost Program Options : argv options
- ankerl : segmented map
- uuid_v4 : create UUIDs with SIMD

Tests:
- nlohmann json
- Boost Beast
- Google test

<br/>

## License
**TODO**

<br/>

## Project Status
The code has been open sourced to gain users and contributors:

### Source
Code improvemements are always possible and of course bug fixes.

### Client APIs
The only client API is the test API in this repo, which is only for testing. Client APIs in friendlier languages are welcome.

