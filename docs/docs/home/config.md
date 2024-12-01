---
sidebar_position: 10
displayed_sidebar: homeSidebar
---

# Configure


## Default Config

There is a default configuration included in the install package and Docker image. 

Default settings:

- Listen on `127.0.0.1:1987`
- Assigned to core `0`
- Sessions disabled
- Persistance disabled


<br/>

```json title="default.jsonc"
{
  "version":5,
  "ip":"127.0.0.1",
  "port":1987,
  "core":0,
  "maxPayload":1024,
  "persist":
  {
    "enabled":false,
    "path":"./data"
  }
}
```

<br/>

---

<br/>

|Param|Type|Description|Required|
|:---|:---:|:---|:---:|
|version|unsigned int|Must be 5|Y|
|core|unsigned int|The core to assign this instance.<br/> If not present or above maximum available, defaults to `0`|N|
|maxPayload|unsigned int|The max size, in bytes, of the WebSocket payload|Y|
|persist|object|Settings for saving/loading keys|Y|


<br/>

## `core`
This is the **logical** core, which may differ from physical cores when hyperthreading is present.<br/>

:::note
- When running multiple instances, remember to change the `port`
- If running in a Docker container, the core(s) available depends on those available for the container.
:::

<br/>

## `persist`

|Param|Type|Description|
|:---|:---:|:---|
|enabled|bool|`true`:<br/>- `KV_SAVE` and `SH_SAVE` available<br/>- `path` must exist<br/><br/>`false`:<br/>-`KV_SAVE` and `SH_SAVE` not available<br/>- `path` is not checked|
|path|string|Path to the directory where data is stored. Must be a directory.<br/>If `enabled` is true, this path must exist.|

See [SH_SAVE](../api/sessions/sh-save) or [KV_SAVE](../api/kv/kv-save) for more.



