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


`st` can be:

- SaveComplete (save complete without error)
- SaveError (save incomplete, error occured)


See [response status](./../Statuses) for status values.

<br/>

## Examples


```json title="Initiate save"
{
  "SH_SAVE":
  {
    "name":"dump"
  }
}
```

<br/>

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