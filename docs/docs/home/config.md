---
sidebar_position: 10
displayed_sidebar: homeSidebar
---

# Configure

The configuration has settings:

|Param|Type|Description|
|:---|:---:|:---|
|version|unsigned integer|Must be 1|
|kv|object|Settings for the WebSocket server|
|session|object|Settings for saving session data so they can be restored on startup|

<br/>

## `kv`

|Param|Type|Description|
|:---|:---:|:---|
|ip|string|IP address of the WebSocket server|
|port|unsigned int|Port of the WebSocket server|
|maxPayload|unsigned int|Max bytes per query. A query larger than this will be rejected.<br/>Absolute min/max are 64 bytes and 2MB.|

<br/>

## `session`

|Param|Type|Description|
|:---|:---:|:---|
|save|object|Settings used by `SH_SAVE`|

### `session::save`

|Param|Type|Description|
|:---|:---:|:---|
|enabled|bool|`true`: the `SH_SAVE` command is available and `path` must exist. <br/>`false`: `SH_SAVE` not available, and `path` is not checked.|
|path|string|Path to the directory where data is stored. Must be a directory.|

See [SH_SAVE](../api/sessions/sh-save) for more.


<br/>

## Default Config

There is a default configuration included in the install package/Docker image:

```json title="default.json"
{
  "version":1,
  "kv":
  {
    "ip":"127.0.0.1",
    "port":1987,
    "maxPayload":1024
  },
  "session":
  {
    "save":
    {
      "enabled":false,
      "path":"./data"
    }
  }
}
```

This listens on `127.0.0.1:1987` with `SH_SAVE` disabled.