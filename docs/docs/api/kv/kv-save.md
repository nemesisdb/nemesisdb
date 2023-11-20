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
- SaveDirWriteFail (failed to create directory/files required before starting)
- SaveStart (command accepted, expect a final response)


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
    "name":"dump"
  }
}
```

Initial response:

```json title="Save accepted"
{
  "KV_SAVE_RSP":
  {
    "name":"dump",
    "st": 120
  }
}
```
Soon afterwards:

```json title="Save complete"
{
  "KV_SAVE_RSP":
  {
    "name":"dump",
    "st": 121
  }
}
```

After this, the data can be found in:

`<savepath>/dump/<timestamp>`

where `<savepath>` is the `kv::save::path` in the server config.