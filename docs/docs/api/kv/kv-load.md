---
sidebar_position: 120
---

# KV_LOAD

:::info
This command is only available when the server mode is `"kv"`. If the server mode is `"kv_sessions"`, use [`SH_LOAD`](../sessions/sh-load).
:::

Loads data from the filesystem at runtime.

- This command is available even if `kv::save::enabled` is `false`
- The data is read from the `kv::save::path` set in the config file


<br/>

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|Data set name, previously used in `KV_SAVE`|Y|

<br/>

## Response

|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|name|string|Dataset name, as used in the request|
|keys|unsigned int|Number of keys loaded|

<br/>


Possible values for `st`:

- LoadError
- LoadComplete
- CommandSyntax 
- ValueTypeInvalid


See [response status](./../Statuses) for status values.

<br/>

## Example

Save data with [`KV_SAVE`](./kv-save):

```json title="Save data"
{
  "KV_SAVE":
  {
    "name":"users"
  }
}
```

If the data is deleted or the server is restarted it can be restored:

```json title="Load Request"
{
  "KV_LOAD":
  {
    "name":"dump1"
  }
}
```

<br/>

```json title="Load Response"
{
  "KV_LOAD_RSP":
  {
    "st": 141,
    "name":"dump1",
    "keys": 50
  }
}
```
