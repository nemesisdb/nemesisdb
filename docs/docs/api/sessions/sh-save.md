---
sidebar_position: 25
---

# SH_SAVE

:::info
This command is only available when sessions are enabled. When sessions are disabled use [`KV_SAVE`](../kv/kv-save).
:::

Saves the data to the filesystem so it can be loaded on startup or at runtime with [`SH_LOAD`](./sh-load).

- The server config must have `persist::enabled` set `true`.
- Data is written to `persist::path` set in the config file


Restored sessions retain their shared and expiry settings. If a session has expiry settings, the expiry time is set to `now + duration`.

<br/>

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|A friendly name for the dataset. The data is saved to a directory with this name. The name is used when loading the data.|Y|
|tkns|array|An array of session tokens to save. If `tkns` is not defined all sessions are saved.|N|

<br/>


If the config file has the persist path set to "/nemesisdb/data" and this is sent:

```json
{
  "SH_SAVE":
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

`SH_SAVE_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|name|string|The name used in the request|
|duration|unsigned int|Duration, in milliseconds, to write the data|


The response is split in two:

- An initial response indicating if the command is accepted
- A final response confirming the save is complete or error during saving


### Initial Response
`st` can be:

- CommandDisabled (save disabled in the config file)
- CommandSyntax (`name` not present or not a string)
- ValueTypeInvalid (`tkns` is present but not an array, or an item in the array is not an unsigned integer)
- SaveDirWriteFail (failed to create directory/files required before starting)
- SaveStart (command accepted, expect a final response)


### Final Response
`st` can be:

- SaveComplete (save complete without error)
- SaveError (save incomplete, error occured)

If the final response status is `SaveError` the data cannot be loaded.

See [response status](./../Statuses) for status values.

<br/>

## Example

### Save All

```json title="Initiate save"
{
  "SH_SAVE":
  {
    "name":"dump"
  }
}
```

Initial response:

```json title="Save accepted"
{
  "SH_SAVE_RSP":
  {
    "name":"dump",
    "st": 120
  }
}
```
Followed by a confirmation:

```json title="Save complete"
{
  "SH_SAVE_RSP":
  {
    "name":"dump",
    "st": 121
  }
}
```

After this, the data can be found in:

`<savepath>/dump/<timestamp>`

where `<savepath>` is the path in the server config.


### Save Particular Sessions

```json title="Initiate saving of three sessions"
{
  "SH_SAVE":
  {
    "name":"save_three_sessions",
    "tkns":
    [
      3349001631311176311,
      6725099647173095979,
      7096468318535971751
    ]
  }
}
```

A response confirming this was accepted is sent followed by a final `SessionComplete`.