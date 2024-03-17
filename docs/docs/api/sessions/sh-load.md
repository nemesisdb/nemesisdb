---
sidebar_position: 30
---

# SH_LOAD
Loads data from the filesystem at runtime as an alternative to loading data at startup.

- The data is read from the `session::save::path` set in the config file
- This command is available even if `session::save::enabled` is `false`

Restored sessions retain their shared and expiry settings. If a session has expiry settings, the expiry time is set to `now + duration`.

<br/>

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|Data set name, previously used in `SH_SAVE`|Y|

<br/>

:::info
If a session is loaded that already exists, the existing session is not changed.
:::

<br/>

## Response

|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|name|string|Dataset name, as used in the request|
|sessions|unsigned int|Number of sessions loaded|
|keys|unsigned int|Number of keys loaded|

<br/>




Possible values for `st`:

- LoadError
- LoadComplete
- CommandSyntax 
- ValueTypeInvalid


See [response status](./../Statuses) for status values.

<br/>

## Examples

Save data (see [`SH_SAVE`](./sh-save)):

```json title="Save all sessions"
{
  "SH_SAVE":
  {
    "name":"dump1"
  }
}
```

To restore:

```json title="Request"
{
  "SH_LOAD":
  {
    "name":"dump1"
  }
}
```

<br/>

```json title="Response"
{
  "SH_LOAD_RSP":
  {
    "name":"dump1",
    "st": 141,
    "sessions": 10,
    "keys": 50
  }
}
```
