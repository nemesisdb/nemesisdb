---
sidebar_position: 1
displayed_sidebar: tutorialSidebar
---

# Overview

- All key value data is persisted (you can't select which keys to save)
- Data can be restored at startup with a command line argument at runtime with the `KV_LOAD` command
- Data is written as raw JSON, a future release will support a more space efficient format

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

<br/>

## Saving
The `KV_SAVE` command is used to save data:

```json
{
  "KV_SAVE":
  {
    "name":"defaults"
  }
}
```

<br/>

## Loading
Data can be loaded at startup with command line args or at runtime with `KV_LOAD`:

- Startup
  - `--loadName` loads from a name previously used in `KV_SAVE`
  - It can be used with `--loadPath` to read data from a path that's different from that in the server config

- Runtime
  - `KV_LOAD` to load at any time
  - `KV_LOAD` will only read data from the path in the server config
