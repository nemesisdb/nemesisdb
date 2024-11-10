---
sidebar_position: 1
displayed_sidebar: apiSidebar
---

# Overview

There are seperate APIs for key-values and sessions, but they all share:

- Command names must be upper case
- Commands are a JSON object
- Commands that return a response contain a status (`st`) which is an unsigned integer
- Parameters are in lower case unless stated otherwise

There is a key value [TLDR](../home/tldr-kv).


## Sessions Disabled
The commands all begin with `KV_`, for example `KV_SET` and `KV_GET`.

```json title='Store keys: username, job and age'
{
  "KV_SET":
  {
    "keys":
    {
      "username":"Bob",
      "job":"Builder",
      "age":26
    }
  }
}
```

And retrieved with:

```json
{
  "KV_GET":
  {
    "keys":["username","job","age"]
  }
}
```

Commonly used commands:

|Command|Purpose|
|---|---|
|KV_SETQ|Same as `KV_SET` but a response is only sent on an error|
|KV_ADD|Stores keys, but unlike `KV_SET`, does not overwrite if a already exists|
|KV_ADDQ|As `KV_ADD` but a response is only sent on an error|
|KV_CONTAINS|Checks if keys exist|
|KV_RMV|Remove/delete keys|

<br/>

## Sessions Enabled
This is similar to when sessions are disabled, except:

- A session must be created with `SH_NEW`
- The token returned by `SH_NEW` must be used in subsequent `KV_` commands

A good place to start is [First Steps](../tutorials/first-steps/setup) which shows how to create a session and store/get data.

```json title='Create Session'
{
  "SH_NEW":
  {
    "name":"user"
  }
}
```

```json title='Create session response'
{
  "SH_NEW_RSP":
  {
    "name":"user1",
    "tkn":12345678910
  }
}
```

After this, you can store data in that session:

```json
{
  "KV_SET":
  {
    "tkn":12345678910,
    "keys":
    {
      "username":"user1",
      "job":"Builder",
      "age":26
    }
  }
}
```

You can send another `SH_NEW` to create a new session, then submit a `KV_SET` with the new session token:

```json title='Create Session'
{
  "SH_NEW":
  {
    "name":"user"
  }
}
```

```json title='Create session response'
{
  "SH_NEW_RSP":
  {
    "name":"user",
    "tkn":6119461887874704569
  }
}
```

Store keys in this new session:

```json
{
  "KV_SET":
  {
    "tkn":6119461887874704569,
    "keys":
    {
      "username":"user2",
      "job":"Plumber",
      "age":61
    }
  }
}
```

In subsequent `KV_` commands, use the appropriate `tkn` to access each session's data.

This returns "user1":

```json
{
  "KV_GET":
  {
    "tkn":12345678910,
    "keys":["username"]
  }
}
```
<br/>

This returns "user2":
```json
{
  "KV_GET":
  {
    "tkn":6119461887874704569,
    "keys":["username"]
  }
}
```
