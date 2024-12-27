---
sidebar_position: 1
displayed_sidebar: apiSidebar
---

# Overview

:::info
During a busy period of changes, the Nemesis API docs have fallen behind, but will be updated soon.

The [Python API](/category/python) docs are current.
:::


- Command names must be upper case
- Commands are a JSON object
- Commands that return a response contain a status (`st`) which is an unsigned integer
- Parameters use camel case


## Key value
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
|KV_ADD|Stores keys, but unlike `KV_SET`, does not overwrite if a already exists|
|KV_CONTAINS|Checks if keys exist|
|KV_RMV|Delete key(s)|
|KV_CLEAR|Deletes all keys|
|KV_CLEAR_SET|Deletes all keys and sets new keys in a single command|

<br/>
