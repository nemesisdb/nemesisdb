---
sidebar_position: 1
displayed_sidebar: tutorialSidebar
---

# Overview

- `KV_SAVE` persists all keys
- Data can be restored at startup with a command line argument
- Data can be restored at runtime with `KV_LOAD`
- Data is written as raw JSON, a future release will support a more space efficient format

<br/>

## Persisting Key Values
The `KV_SAVE` command persists key values.

The command saves all keys, you cannot specify particular keys to persist.

```json
{
  "KV_SAVE":
  {
    "name":"stats"
  }
}
```

And later load at runtime with:
```json
{
  "KV_LOAD":
  {
    "name":"stats"
  }
}
```


<br/>

## Loading

- Startup
  - `--loadName` will load data. The name must match a `name` used in `KV_SAVE`
  - `--loadPath` is used to read data from a path that's different from that in the server config
    - Only applies at startup, not with `KV_LOAD`

- Runtime
  - `KV_LOAD` offer the flexibility to load data at any time
  - `KV_LOAD` only read data from the persist path in the server config


<br/>

## Persistance Structure
Data is saved with a name and a timestamp. The name is used when loading as a way to identify which data to restore.

The server's config file has a setting for the save path, in to which the data is written.

The structure is:

```bash
└── <name>
    └── <timestamp>
        ├── data
        └── md
```

|Directory|Purpose|
|:---|:---|
|name|The name used in the save command|
|timestamp|Timestamp when the data was saved|
|data|Contains the data|
|md|Contains metadata|

<br/>

This structure allows you to save and restore specific data in a future release.

For example, if we save data using the name `defaults` we will have:

```bash
.
└── defaults
    └── 1701060607618993128
        ├── data
        └── md
```

If we send another save command with the same name:

```bash
└── defaults
    ├── 1701060607618993128
    │   ├── data
    │   └── md
    └── 1701061180908039731
        ├── data
        └── md
```

:::info
The current version does not allow you to load a specific timestamp, it always selects the most recent. <br/>

If you need to restore specific data, use a different name for each use of `KV_SAVE`.
:::
