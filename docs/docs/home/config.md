---
sidebar_position: 10
displayed_sidebar: homeSidebar
---

# Configure

The configuration has settings:

|Param|Type|Description|
|:---|:---:|:---|
|version|unsigned integer|Must be 2|
|mode|string|"kv" for key-value, "ts" for timeseries|
|kv|object|Settings for key-value|
|ts|object|Settings for timeseries|

<br/>

# `kv`

|Param|Type|Description|
|:---|:---:|:---|
|ip|string|IP address of the WebSocket server|
|port|unsigned int|Port of the WebSocket server|
|maxPayload|unsigned int|Max bytes per query. A query larger than this will be rejected.<br/>Absolute min/max are 64 bytes and 2MB.|

<br/>

## `save`

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
|maxPayload|unsigned int|Max bytes per query. A query larger than this will be rejected.<br/>Absolute min/max are 64 bytes and 2MB.|


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
  "version":2,
  "mode":"kv",
  "kv":
  {
    "ip":"127.0.0.1",
    "port":1987,
    "maxPayload":1024,
    "session":
    {
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
