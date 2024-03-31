---
sidebar_position: 10
displayed_sidebar: homeSidebar
---

# Configure


## Default Config

There is a default configuration included in the install package and Docker image. 

Default settings:

- Assigned to core `0`
- Mode: key value (`kv`)
  - Sessions disabled
  - Save disabled
- Listen on `127.0.0.1:1987`

<br/>

```json title="default.json"
{
  "version":4,
  "mode":"kv",
  "core":0,
  "ip":"127.0.0.1",
  "port":1987,
  "maxPayload":1024,  
  "kv":
  {
    "save":
    {
      "enabled":false,
      "path":"./data"
    }
  },
  "kv_sessions":
  {
    "save":
    {
      "enabled":false,
      "path":"./data"
    }
  },
  "ts":
  {
    
  }
}
```

<br/>

---

<br/>

|Param|Type|Description|Required|
|:---|:---:|:---|:---:|
|version|unsigned int|Must be 4|Y|
|mode|string|"kv" for key value<br/>"kv_sessions" for key value with sessions<br/>"ts" for timeseries|Y|
|core|unsigned int|The core to assign this instance.<br/> If not present or above maximum available, defaults to `0` (the first core)|N|
|kv|object|Settings for key value.<br/>Required if mode is `"kv"`.|N|
|kv_sessions|object|Settings for key value with sessions.<br/>Required if mode is `"kv_sessions"`.|N|
|ts|object|Empty but required if mode is `"ts"`.|N|


<br/>

# `core`
This is the **logical** core, which may differ from physical cores when hyperthreading is present.<br/>

Use `lscpu | grep "CPU(s):"` to find the logical core count.

Core assignment is useful when running multiple instances on the same physical server. If so then ensure the instances are on different ports.

:::note
If running in a Docker container, the core(s) available depends on those available for the container.
:::

<br/>

# `kv`

|Param|Type|Description|
|:---|:---:|:---|
|save|object|Settings for persisting kv data. Only used when sessions are disabled.|

<br/>

# `session`
|Param|Type|Description|
|:---|:---:|:---|
|save|object|Settings for persisting session data|

## `save`

|Param|Type|Description|
|:---|:---:|:---|
|enabled|bool|`true`:<br/>- `KV_SAVE` available (when `mode` is `kv`)<br/>- `SH_SAVE` available (when `mode` is `kv_sessions`)<br/>- `path` must exist<br/><br/>`false`:<br/>-`SH_SAVE` and `KV_SAVE` not available<br/>- `path` is not checked.|
|path|string|Path to the directory where data is stored. Must be a directory.<br/>If `enabled` is true, this path must exist.|

See [SH_SAVE](../api/sessions/sh-save) or [KV_SAVE](../api/kv/kv-save) for more.

<br/>

# `ts`
No settings but must be present and an empty object if `mode` is `"ts"`.


<br/>


