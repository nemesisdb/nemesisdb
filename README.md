# NemesisDB
NemesisDB is an in-memory JSON database, supporting key-value and timeseries data:

- All data and commands are JSON over WebSockets
- Commands are submitted via a WebSocket

<br/>

# Time Series
Data is stored by creating an array of times and an array of events.

With temperature readings to store:

|Time|Temperature|
|:---:|:---:|
|10|20|
|11|21|
|12|25|
|13|22|
|14|23|


Create a time series:

```json
{
  "TS_CREATE":
  {
    "name":"temperatures",
    "type":"Ordered"
  }
}
```

- `name`: name of the series
- `type`: only "Ordered" available at the moment

<br/>

Add five events:

```json
{
  "TS_ADD_EVT":
  {
    "ts":"temperatures",
    "t":[10,11,12,13,14],
    "evt":
    [
      {"temperature":20},
      {"temperature":21},
      {"temperature":25},
      {"temperature":22},
      {"temperature":23}
    ]
  }
}
```

- `ts` : the name of time series
- `t` : time values
- `evt` : event objects


Get event data between times `10` and `12` inclusive:

```json
{
  "TS_GET":
  {
    "ts":"temperatures",
    "rng":[10,12]
  }
}
```

- `rng` is "time range" with min and max inclusive
- `rng` can be empty (`[]`) to get whole time range

<br/>

If an event member is indexed, it can be used in a `where` clause.

Get events where `temperature` is greater than or equal to 23:

```json
{
  "TS_GET":
  {
    "ts":"temperatures",
    "rng":[],
    "where":
    {
      "temperature":
      {
        ">=":23
      }
    }
  }
}
```

- Returning data for times `12` and `14`


The range `[]` operator is used in `where` terms to limit the temperature value to between `21` and `22` inclusive: 

```json
{
  "TS_GET":
  {
    "ts":"temperatures",
    "rng":[],
    "where":
    {
      "temperature":
      {
        "[]":[21,22]
      }
    }
  }
}
```

- Returning data for times `11` and `13`

<br/>

More information [here](https://docs.nemesisdb.io/home/tldr-ts).

<br/>
<br/>

# Key Value
Rather than one large map, key-values are split into sessions:

- Each session has a dedicated map
- A session can live forever or expire after a given time
- When a session expires its data is always deleted, and optionally the session can be deleted

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

You can create as many sessions as required (within memory limitations). When a session is created, a session token is returned (an integer), so to switch between sessions only requires using the appropriate token.

More info [here](https://docs.nemesisdb.io/tutorials/sessions/what-is-a-session).

<br/>
<br/>

# Install
NemesisDB is available as a Debian package or Docker image:

- Package:  [Releases](https://github.com/nemesisdb/nemesisdb/releases) 
- Docker: [Docker Hub](https://hub.docker.com/r/nemesisdb/nemesisdb/tags)

You can also build from source, instructions below.


<br/>
<br/>

# Design

<br/>

## Time Series
This uses parallel vectors to store data:

- Two vectors, `m_times` and `m_events`, store the times and events
- An event at `m_events[i]` occured at `m_times[i]`

If we have these values:

|Time|Temperature|
|:---:|:---:|
|10|5|
|15|4|
|20|2|
|25|6|

This can be visualised as:


![TimeSeries Design](https://20aac7f3a5b7ba27bcb45d6ccf5d4c71.cdn.bubble.io/f1710001659309x923605728983864200/tldr-ts-parallel.svg?_gl=1*1biw9dg*_gcl_au*MTc4NTg0NDIyMy4xNzA3NDIwNzA2*_ga*MTcwMTY5ODQzNC4xNjk3NTQyODkw*_ga_BFPVR2DEE2*MTcxMDAwMTYxOC4yNi4xLjE3MTAwMDE2MzkuMzkuMC4w)


The first release of time series support is single threaded. 

<br/>

## Key Value
The design separates I/O and session data threads:

- Core(s) are dedicated for I/O operations (the WebSocket server)
- Remaining core(s) are dedicated to handle sessions

Each thread is assigned to a core.

With 4 cores, 3 are assigned to I/O and 1 for session data:

![4 cores](https://20aac7f3a5b7ba27bcb45d6ccf5d4c71.cdn.bubble.io/f1707487526533x999369528600104800/nemesis_cores_4.png?_gl=1*b29xz5*_gcl_au*MTc4NTg0NDIyMy4xNzA3NDIwNzA2*_ga*MTcwMTY5ODQzNC4xNjk3NTQyODkw*_ga_BFPVR2DEE2*MTcwNzQ4Njc4MS4yNC4xLjE3MDc0ODc2MDIuNjAuMC4w)

<br/>

With 6 cores, 4 are assigned to I/O and 2 for session data:

![6 cores](https://20aac7f3a5b7ba27bcb45d6ccf5d4c71.cdn.bubble.io/f1707487522110x648951654930151300/nemesis_cores_6.png?_gl=1*j7jxla*_gcl_au*MTc4NTg0NDIyMy4xNzA3NDIwNzA2*_ga*MTcwMTY5ODQzNC4xNjk3NTQyODkw*_ga_BFPVR2DEE2*MTcwNzQ4Njc4MS4yNC4xLjE3MDc0ODc2MDIuNjAuMC4w)


In the second image, there are two session map threads, each managing a subset of the sessions. There is no data shared between the session map threads so they process concurrently.


> [!NOTE]
> There is a 4 thread max, so at most there will be 3 I/O threads and 1 session worker thread.
> - The limit is set by `NEMESIS_MAX_CORES` in `NemesisCommon.h`
> - The ratio of I/O to session pool threads is defined by `CoresToIoThreads` map in `KvServer::init()`. This defines how many threads are assigned to I/O, the remaining are for session pool(s).
>
> Most testing has been with limit as 4


<br/>
<br/>

# Build - Linux Only
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

# Run
1. `cd server/Release/bin`
2. `./nemesisdb --config=../../configs/default.json`
    - Server WebSocket listening on `127.0.0.1:1987` (defined in `default.json`)
3. `ctrl+c` to exit

<br/>

# Key Value Commands
The quickest way to get started is to use software such as [Postman](https://www.postman.com/downloads/) to query the WebSocket

The [first steps](https://docs.nemesisdb.io/tutorials/first-steps/setup) guide is a good place to start.


## Create Session
```json
{
  "SH_NEW":
  {
    "name":"session1"
  }
}
```
This returns the session token `tkn`, for example: `448892247316960382` .


## Store String Keys
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

## Store Object Key
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

## Get Keys
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

# Code
The server binary implementation is `server/server.cpp`:

1. `server.cpp` checks the command line params, the config and then calls `core/Kv/KvServer::run()`
2. `KvServer::run()` runs the WebSocket server
3. When a message arrives, the `.message` handler is called, which parses the JSON. If valid, it calls `KvHandler::handle()`
4. `KvHandler` validates the command then submits it to a `KvPoolWorker`
5. The session token is used to determine the pool worker responsible for its session data
6. The command is pushed to pool worker's channel, which pops and executes the command

<br/>

# External Libraries
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

# License
See LICENSE file.


