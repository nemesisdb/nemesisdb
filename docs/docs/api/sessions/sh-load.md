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

If a name does not exist the response is in form:

```json title="Dataset does not exist"
{
  "SH_LOAD_RSP":
  {
    "st": 142,
    "m": "<name> does not exist"
  }
}
```
Where `<name>` is the name that does not exist.

<br/>

Otherwise, the response is in form:

```json
{
  "SH_LOAD_RSP":
  {
    "<name1>":
    {
      "st":<status>,
      "keys":<keys>,
      "sessions":<sessions>,
      "m":<msg>
    },
    "<name2>":
    {
      "st":<status>,
      "keys":<keys>,
      "sessions":<sessions>,
      "m":<msg>
    },
    "<nameN>":
    {
      "st":<status>,
      "keys":<keys>,
      "sessions":<sessions>,
      "m":<msg>
    }
  }
}
```
<br/>

|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|sessions|unsigned int|Number of sessions loaded|
|keys|unsigned int|Number of keys loaded|
|m|string|Error message. Empty if `st` is `LoadComplete`|

<br/>

Possible values for `st`:

- CommandSyntax (`names` not present or not an array)
- ValueTypeInvalid (member of `names` is not a string)
- LoadError (`m` contains error message)
- LoadComplete (`m` is empty)


See [response status](./../Statuses) for status values.

<br/>

## Examples

### Single Dataset
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

### Multiple Datasets

```json title="dump2 exists but has no data"
{
  "SH_LOAD":
  {
    "names":["dump1", "dump2"]
  }
}
```

An empty dataset is not an error:

```json title="Response"
{
  "SH_LOAD_RSP": {
    "dump1": {
      "m": "",
      "sessions": 10,
      "keys": 50,
      "st": 141
    },
    "dump2": {
      "m": "",
      "sessions": 0,
      "keys": 0,
      "st": 141
    }
  }
}
```