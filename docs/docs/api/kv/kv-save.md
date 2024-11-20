---
sidebar_position: 110
---

# KV_SAVE

:::info
This command is only available when sessions are disabled. When sessions are enabled use [`SH_SAVE`](../sessions/sh-save).
:::

Saves the data to the filesystem so it can be loaded on startup or at runtime with [`KV_LOAD`](./kv-load).

- The server config must have `persist::enabled` set `true`.
- Data is written to `persist::path` set in the config file


<br/>

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|A friendly name for the dataset. The data is saved to a directory with this name. The name is used when loading the data.|Y|

<br/>

The data is written to: `<persist::path>/<name>/<timestamp>`.

If the config has `"persist::path"` set to `"/nemesisdb/data"` and the command is:

```json
{
  "KV_SAVE":
  {
    "name":"dump1"
  }
}
```

The data is written to:  `/nemesisdb/data/dump1/<timestamp>`.

If the command is sent again with the same `name`, it won't overwrite the first because the `<timestamp>` is different.

When loading data, the newest timestamp is selected. 

<br/>

## Response

`KV_SAVE_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|name|string|The name used in the request|
|duration|unsigned int|Duration, in milliseconds, for the save to complete|


`st` can be:

- SaveComplete (save complete without error)
- SaveError (save incomplete, error occured)

See [response status](./../Statuses) for status values.

<br/>

## Examples

```json title="Initiate save"
{
  "KV_SAVE":
  {
    "name":"users"
  }
}
```


```json title="Save complete"
{
  "KV_SAVE_RSP":
  {
    "st": 121,
    "name":"users",
    "duration":50
  }
}
```

