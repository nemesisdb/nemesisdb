---
sidebar_position: 110
---

# KV_SAVE

:::info
This command is only available when the server mode is `"kv"`. If the server mode is `"kv_sessions"`, use [`SH_SAVE`](../sessions/sh-save).
:::

Saves the data to the filesystem so it can be loaded on startup or at runtime with [`KV_LOAD`](./kv-load).

- `kv::save::enabled` must be `true` for this command to be available
- The data is written to the `kv::save::path` set in the config file


Restored sessions retain their shared and expiry settings. If a session has expiry settings, the expiry time is set to `now + duration`.

<br/>

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|A friendly name for the dataset. The data is saved to a directory with this name. The name is used when loading the data.|Y|

<br/>

The data is written to: `<kv::save::path>/<name>/<timestamp>`.

If the config has `"kv::save::path"` set to `"/nemesisdb/data"` and the command is:

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


There may be two responses:

1. Initial: indicates if the command is accepted
   - If this `st` is SaveStart, it is followed by a second response when the save is complete/failed
   - If this `st` is any other value, a second response is not sent
2. Final: confirms the save is complete or an error occured  



### Initial Response
`st` can be:

- SaveStart (command accepted, expect a final response)
- CommandDisabled (save disabled in the config file)
- CommandSyntax 
- ValueTypeInvalid
- SaveDirWriteFail (failed to create directory/files required before starting)


### Final Response
`st` can be:

- SaveComplete (save complete without error)
- SaveError (save incomplete, error occured)

See [response status](./../Statuses) for status values.

<br/>

## Example

```json title="Initiate save"
{
  "KV_SAVE":
  {
    "name":"users"
  }
}
```

Initial response:

```json title="Save accepted"
{
  "KV_SAVE_RSP":
  {
    "name":"users",
    "st": 120
  }
}
```

Final response:

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

