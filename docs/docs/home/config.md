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
  "version":6,
  "core":0,
  "ip":"127.0.0.1",
  "port":1987,
  "maxPayload":8192,
  "persist":
  {
    "enabled":false,
    "path":"./data"
  },
  "arrays":
  {
    "maxCapacity":10000,
    "maxResponseSize":10000
  },
  "lists":
  {
    "maxResponseSize":10000
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
|arrays|object|Settings for arrays|Y|
|lists|object|Settings for lists|Y|


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
|enabled|bool|`true`:<br/>- `KV_SAVE` available<br/>- `path` must exist<br/><br/>`false`:<br/>-`KV_SAVE` not available<br/>- `path` is not checked|
|path|string|Path to the directory where data is stored. Must be a directory.<br/>If `enabled` is true, this path must exist.|

See [KV_SAVE](../api/kv/kv-save) for more.

<br/>

## arrays

|Param|Type|Description|
|:---|:---:|:---|
|maxCapacity|unsigned int|Arrays are fixed size so this is the max number of elements in the array, __not__ max number of bytes.|
|maxResponseSize|unsigned int|Maximum number of items permitted in a response that returns multiple items, such as `get_range()`.<br/>If this is less than `maxCapacity` it is possible that not all items will be returned|

<br/>

## lists

|Param|Type|Description|
|:---|:---:|:---|
|maxResponseSize|unsigned int|Maximum number of items permitted in a response that returns multiple items|