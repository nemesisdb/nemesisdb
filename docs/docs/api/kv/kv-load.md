---
sidebar_position: 120
---

# KV_LOAD
Loads data from the filesystem at runtime.

- The data is read from the `persist::path` set in the config file


<br/>

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|name|string|Data set name, previously used in `KV_SAVE`|Y|

:::info
This command is available even if persistence is disabled.
:::


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
