---
sidebar_position: 1
displayed_sidebar: tutorialSidebar
---

# Overview

- `SH_SAVE` persist all sessions or specific session(s)
- `KV_SAVE` persists all keys
- Data can be restored at startup with a command line argument
- Data can be restored at runtime with the `SH_LOAD` or `KV_LOAD`
- Data is written as raw JSON, a future release will support a more space efficient format

<br/>

## Saving Key Value
The `KV_SAVE` command persist key values (i.e. those not in a session).

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

## Saving Sessions
The `SH_SAVE` command is used to persist session(s).

Save all sessions:

```json title='Save all sessions'
{
  "SH_SAVE":
  {
    "name":"users"
  }
}
```

Supply tokens (`tkns`) to save particular sessions:

```json title="Save two sessions"
{
  "SH_SAVE":
  {
    "name":"blocked_users",
    "tkns":[123456, 654321]
  }
}
```


<br/>

## Loading

- Startup
  - `--loadName` will load data. The name must match a `name` used in either `KV_SAVE` or `SH_SAVE`
  - `--loadPath` is used to read data from a path that's different from that in the server config
    - Only applies at startup, not with `KV_LOAD` or `SH_LOAD`

- Runtime
  - `SH_LOAD` and `KV_LOAD` offer the flexibility to load data at any time
  - `SH_LOAD` and `KV_LOAD` only read data from the persist path in the server config


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
|data|Contains the session data|
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

If you need to restore specific data, use a different name for each use of `SH_SAVE`/`KV_SAVE`.
:::
