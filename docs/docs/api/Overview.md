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


## Key value
The commands all begin with `KV_`, for example `KV_SET` and `KV_GET`. They store keys that are not in a session (independent keys).

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
|KV_ADD|Stores keys, but unlike `KV_SET`, does not overwrite if a already exists|
|KV_CONTAINS|Checks if keys exist|
|KV_RMV|Remove/delete keys|

<br/>

## Sessions
The session API contains commands to manage sessions, such as `SH_NEW`, `SH_END`, etc.

- Create a session with `SH_NEW`
- The token returned by `SH_NEW` can be used in subsequent `SH_` commands that require it
- Use `SH_SET`, `SH_GET` etc to set and get keys. These are the same as their `KV_` counterparts but require a token (`tkn`)


```json title='Create Session'
{
  "SH_NEW":{}
}
```

```json title='Create session response'
{
  "SH_NEW_RSP":
  {
    "tkn":16351792548006066062
  }
}
```

After this, you can store data in that session:

```json
{
  "SH_SET":
  {
    "tkn":16351792548006066062,
    "keys":
    {
      "username":"user1",
      "job":"Builder",
      "age":26
    }
  }
}
```

See [SH_NEW](sessions/sh-new.md) for details.