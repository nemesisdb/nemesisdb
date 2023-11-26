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
|names|array of strings|Dataset names as previously used in `SH_SAVE`|Y|

<br/>

- If a session is loaded that already exists, the existing session is not changed

<br/>

## Response

`SH_LOAD_RSP`

The response is in form:

```json
{
  "SH_LOAD_RSP":
  {
    "<loadName1>":
    {
      "st":<status>,
      "keys":<keys>,
      "sessions":<sessions>
    }
  }
}
```

|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|sessions|unsigned int|Number of sessions restored|
|keys|unsigned int|Number of keys restored|


- CommandSyntax (`names` not present or not an array)
- ValueTypeInvalid (member of `names` is not a string)
- LoadError (Data path or load name not found)
- LoadComplete


See [response status](./../Statuses) for status values.

<br/>

## Example

With a database of data we save all sessions:

```json
{
  "SH_SAVE":
  {
    "name":"dump1"
  }
}
```

Some time later after a restart or the data has been deleted:

```json title="Restore"
{
  "SH_LOAD":
  {
    "names":["dump1"]
  }
}
```

```json title="Response"
{
  "SH_LOAD_RSP":
  {
    "dump1":
    {
      "st": 141,
      "sessions": 10,
      "keys": 50
    }
  }
}
```