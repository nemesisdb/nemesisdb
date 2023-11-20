---
sidebar_position: 110
---

# KV_SAVE
Saves the data to the filesystem so it can be loaded on startup, restoring the database.


- The data is written to the `kv::save::path` set in the config file
- `kv::save::enabled` must be `true` for this command to be available

<br/>

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|A friendly name for the dataset. The data is saved to a directory with this name. The name is used when loading the data.|Y|


:::info
The whole database is written by `KV_SAVE`, there is not an option to only write what has changed since the last save. A future version will offer this.
:::

<br/>

If the config file has:

```json
"save":
{
  "enabled":true,
  "path":"/nemesisdb/data"
}
```
And this is sent:

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

See [response status](./../Statuses) for status values.

|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|name|string|The name used in the request.|

This response split in two:

- An initial response indicating the command is accepted
- A final response confirming the save is complete


### Initial Response
`st` can be:

- CommandDisabled (command disabled in the config file)
- CommandSyntax (`name` not present or not a string)
- SaveDirWriteFail (failed to create directory/file)
- SaveStart


### Final Response
`st` can be:

- SaveComplete to confirm the save is finished


## Example

```json title="Initiate save"
{
  "KV_SAVE":
  {
    "name":"first_save"
  }
}
```

Intial response:

```json title="Save start"
{
  "KV_SAVE_RSP":
  {
    "name":"first_save",
    "st": 120
  }
}
```
Soon afterwards:

```json title="Save complete"
{
  "KV_SAVE_RSP":
  {
    "name":"first_save",
    "st": 121
  }
}
```