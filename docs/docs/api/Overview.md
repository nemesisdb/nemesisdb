---
sidebar_position: 1
displayed_sidebar: apiSidebar
---

# Overview

There are seperate APIs for time series, key-values and sessions they all share:

- Command names must be upper case
- Commands are a JSON object
- Commands which return a response contain a status (`st`) which is an unsigned integer
- Parameters are in lower case unless stated otherwise

There is a TLDR for [time series](../home/tldr-ts) and [key value](../home/tldr-kv).
<br/>

## Time Series
These are to create and delete time series, add events and search for events.

Use [`TS_CREATE`](ts/ts-create-series) after which you add events with [`TS_ADD_EVT`](ts/ts-add-evt) and get/search events with [`TS_GET`](ts/ts-get).

To use `where` in `TS_GET` or `TS_GET_MULTI` you must index event members with [`TS_CREATE_INDEX`](ts/ts-create-index).

<br/>

## Key Value
These store, update, find and retrieve session data. They start `KV_`.

Key-value can be with or without sessions.


### Sessions Enabled

- Each session has a dedicated key-value map
- Key names must only be unique within a session (because each session has a dedicated map)
- Each use of a `KV_` command must include a session token (to identify which session this command is for)
- A session token is created by calling `SH_NEW` which returns a token

A good place to start is [First Steps](../tutorials/first-steps/setup) which shows how to create a session and store/get data.

```json title='Create Session'
{
  "SH_NEW":
  {
    "name":"my session"
  }
}
```

```json title='Create session response'
{
  "SH_NEW_RSP":
  {
    "name":"my session",
    "tkn":12345678910
  }
}
```

After this, you can store data in that session:

```json title='Store keys: username, job and age'
{
  "KV_SET":
  {
    "tkn":12345678910,
    "keys":
    {
      "username":"Bob",
      "job":"Builder",
      "age":26
    }
  }
}
```

- This stores three keys in that session, `username`, `job` and `age` which are of type string, string and integer respectively

To switch session, just change the token (`tkn`).


### Sessions Disabled

- A single map stores all keys
- No session token required in each `KV_` command
- Lower memory usage and latency

