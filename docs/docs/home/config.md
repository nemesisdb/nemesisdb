---
sidebar_position: 2
displayed_sidebar: homeSidebar
---

# Configure

The configuration has paremeters:

|Param|Type|Description|
|:---|:---:|:---|
|version|unsigned integer|Must be 1|
|kv|object|Contains settings for the WebSocket server|

<br/>

`kv`:

|Param|Type|Description|
|:---|:---:|:---|
|ip|string|IP address to bind the WebSocket server|
|port|unsigned int|Port of the WebSocket server|
|maxPayload|unsigned int|Max bytes per query. A query larger than this will be rejected.<br/> Max is 2MB.|


## Default Config

There is a default configuration included in the install package:

```json
{
  "version":1,
  "kv":
  {
    "ip":"127.0.0.1",
    "port":1987,
    "maxPayload":1024
  }
}
```

This listens on `127.0.0.1:1987`.