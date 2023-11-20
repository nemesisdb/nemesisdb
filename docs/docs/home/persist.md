---
sidebar_position: 25
displayed_sidebar: homeSidebar
---

# Persist Data
NemesisDB is an in-memory/cache database meaning all the data is always stored in RAM. This offers performance benefits by removing calls to the filesystem and reducing complexity, but it also means if the server shutdowns all the data is lost.

To help this, the data can be persisted to the filesystem using the [SH_SAVE](../api/sessions/sh-save) command. This command writes all data to file so it can be restored on startup.


The `SH_SAVE` has a server config section, which defaults to:

```json
"save":
{
  "enabled":false,
  "path":"./data"
}
```

- When `enabled` is false, the `path` is not checked so does not need to exist and `kV_SAVE` is disabled
- When `enabled` is true, the `path` must exist and be a directory



## Data Structure
The `SH_SAVE` command requires a `name`:

- A directory with this name is created in `save::path`
- Inside `name`, a directory with the current timestamp is created, in which the data is written

The purpose of creating separate directories is to allow separate save points so you can restore from a known point.

For example, if we send:

```json
{
  "SH_SAVE":
  {
    "name":"dump1"
  }
}
```

A folder is created under `dump1`:

```bash title="First save"
└── dump1
    └── 1700435579410403857
```

Send another `SH_SAVE` with the same `name`:

```bash title="Second save"
└── dump1
    ├── 1700435579410403857
    └── 1700435945763630730
```

A new directory is created for the second save.

<br/>

## Restore Data
Data is loaded using the name and the newest data is chosen. The longer term aim is to provide a method to select the data based on the name and timestamp.


:::note
If you want to store from a particular point, use a separate name for each use of `SH_SAVE` until selecting by timestamp is implemented.
:::

Restoring data is only available during startup, using the `--loadName` switch:

```bash
./nemesisdb --config=default.json --loadName=dump1
```

<br/>

## Performance
Save and restore performance is mostly governed by disk i/o performance.

A database with 1M sessions, with each session containing 5 keys (so 5M keys total), totalling ~385MB of data on an NVME disk:

- Save: 4 seconds
- Load: 7 seconds


:::info
Save and load is a new feature, added in version 0.3.4 (Nov 2023) so improvements will follow.
New features will be added such as:

- Select to restore data from name and timestamp
- A `SH_LOAD` command to restore data at runtime
- `SH_SAVE` can persist individual sessions rather than all sessions
:::