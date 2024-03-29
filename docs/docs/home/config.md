---
sidebar_position: 10
displayed_sidebar: homeSidebar
---

# Configure

|Param|Type|Description|Required|
|:---|:---:|:---|:---:|
|version|unsigned int|Must be 3|Y|
|mode|string|"kv" for key value<br/> "ts" for timeseries|Y|
|core|unsigned int|The core to assign this instance.<br/> If not present or above maximum available, defaults to `0` (the first core)|N|
|kv|object|Settings for key value. Required if mode is `"kv"`.|N|
|ts|object|Settings for timeseries. Required if mode is `"ts"`.|N|

<br/>

# `core`
This is the **logical** core, which may differ from physical cores when hyperthreading is present.<br/>

Use `lscpu | grep "CPU(s):"` to find this value.

Core assignment is useful when running multiple instances on the same physical server. If so then ensure the instances are on different ports.

:::note
If running in a Docker container, the core(s) available depends on those available for the container.
:::

<br/>

# `kv`

|Param|Type|Description|
|:---|:---:|:---|
|ip|string|IP address of the WebSocket server|
|port|unsigned int|Port of the WebSocket server|
|maxPayload|unsigned int|Max bytes per query. A query larger than this will be rejected.<br/>Absolute min/max are 64 bytes and 8Kb.|
|session|object|Settings for session saving (details below)|

<br/>

# `session`
|Param|Type|Description|
|:---|:---:|:---|
|enabled|bool|`true`: Sessions enabled. Use `SH_NEW` to create a session and a token which must be used in subsequent `KV_` commands.<br/><br/>`false`: Sessions disabled, use `KV_` commands without session token.|
|save|object|Settings for persisting session data|

## `session::save`

|Param|Type|Description|
|:---|:---:|:---|
|enabled|bool|`true`: the `SH_SAVE` command is available and `path` must exist. <br/>`false`: `SH_SAVE` not available, and `path` is not checked.|
|path|string|Path to the directory where data is stored. Must be a directory.<br/>If `enabled` is true, this path must exist|

See [SH_SAVE](../api/sessions/sh-save) for more.

<br/>

# `ts`

|Param|Type|Description|
|:---|:---:|:---|
|ip|string|IP address of the WebSocket server|
|port|unsigned int|Port of the WebSocket server|
|maxPayload|unsigned int|Max bytes per query. A query larger than this will be rejected.<br/>Absolute min/max are 64 bytes and 8Kb.|


<br/>

## Default Config

There is a default configuration included in the install package and Docker image. 

Default settings:

- KV mode
- Listen on `127.0.0.1:1987`
- `SH_SAVE` disabled (`kv:save::session::path` does not need to exist)

<br/>

```json title="default.json"
{
  "version":3,
  "mode":"kv",
  "kv":
  {
    "ip":"127.0.0.1",
    "port":1987,
    "maxPayload":1024,
    "session":
    {
      "enabled":false,
      "save":
      {
        "enabled":false,
        "path":"./data"
      }
    }
  },
  "ts":
  {
    "ip":"127.0.0.1",
    "port":1987,
    "maxPayload":1024
  }
}

```
