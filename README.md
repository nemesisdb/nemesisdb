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

## Sessions Disabled

- There is one map for all keys
- No need to create sessions to store data
- Data is not segregated so keys must be unique over the entire database
- Lower memory usage and higher throughput

## Sessions Enabled
Rather than one large map, key-values are split into sessions:

- Each session has a dedicated map
- A session can live forever or expire after a given time
- When a session expires its data is always deleted, and optionally the session can be deleted

Examples of sessions:
- Each user that logs into an app
- Each connected device in monitoring software
- When an One Time Password is created
- Whilst a user is completing a multi-page online form

<br/>

### Sessions
The purpose of sessions are:
- Each session only contains data required for that session, rather than a single large map
- When accessing (get, set, etc) data, only the data for a particular session is accessed
- Controlling key expiry is simplified because it is sessions that expire, not individual keys

You can create as many sessions as required (within memory limitations). When a session is created, a session token is returned (an integer), so to switch between sessions only requires using the appropriate token.

More info [here](https://docs.nemesisdb.io/tutorials/sessions/what-is-a-session).

<br/>
<br/>

# Install
NemesisDB is available as a Debian package and Docker image:

- Package:  [Releases](https://github.com/nemesisdb/nemesisdb/releases) 
- Docker: [Docker Hub](https://hub.docker.com/r/nemesisdb/nemesisdb/tags)

You can also build from source on Linux, instructions below.


<br/>
<br/>

# Design

As of version 0.5, the engine is single threaded to reduce and simplify code. The thread is assigned to core 0, a future release will this configurable to manage multiple instances on multicore CPUs.

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


<br/>

## Key Value

The structure of JSON is used to determine value types, i.e.:

```json
{
  "KV_SET":
  {
    "keys":
    {
      "username":"James",
      "age":35,
      "address":
      {
        "city":"Paris"
      }
    }
  }
}
```

Set three keys:
- `username` of type string
- `age` of type integer
- `address` of type object


### Save and Restore
Sessions support saving and restoring data:

- Use `SH_SAVE` to write session data to file, either all sessions or particular sessions
- Use `SH_LOAD` to load data from file at runtime
- Use `--loadName` at the command line to load during start up


<br/>

> [!IMPORTANT]
> Data persistance is only implemented when sessions are enabled, a future release will support both.

<br/>
<br/>

# Build - Linux Only
1. Clone via SSH with submodules: `git clone --recursive git@github.com:nemesisdb/nemesisdb.git`
2. Prepare and grab vcpkg libs: `cd nemesisdb && ./prepare_vcpkg.sh`
3. With VS Code (assuming you have C/C++ and CMake extensions):
    - `code .`
    - Select kit (only tested with GCC 12.3.0)
    - Select Release variant
    - Select target as nemesisdb
    - Build
4. Binary is in `server/Release/bin`

<br/>

## Run
Start listening on `127.0.0.1:1987` in KV mode (default in `default.json`)

`./nemesisdb --config=../../configs/default.json`



Use `ctrl+c` to exit


<br/>
<br/>

# External Libraries
Externals are either GitHub submodules or managed by [vcpkg](https://vcpkg.io/en/).

Server:
- uWebsockets : WebSocket server
- jsoncons : json
- plog : logging
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


