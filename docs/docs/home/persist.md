---
sidebar_position: 25
displayed_sidebar: homeSidebar
---

# Persist Data
NemesisDB is an in-memory/cache database meaning all the data is always stored in RAM. This offers performance benefits but it also means if the server shutdowns all the data is lost.

Key value data can be persisted to the filesystem and restored on startup or at runtime.

More details are available in [Persist](/tutorials/persist-data/overview).

<br/>

## Save
Data is saved to the filesystem using `SH_SAVE`:

```json
{
  "SH_SAVE":
  {
    "name":"mydata"
  }
}
```

Data is saved to `session::save::path`. 

This commands saves all sessions but you can also save specific sessions by setting tokens:

```json
{
  "SH_SAVE":
  {
    "name":"mydata",
    "tkns":[782729387182638, 1183740548782]
  }
}
```

This saves the data for the two sessions with those session tokens.

<br/>

## Restore: Startup

To restore data at startup use the `--loadName` switch:

```bash
./nemesisdb --config=default.json --loadName=mydata
```

This expects to find `mydata` in the `session::save::path` set in the server config. This can be overridden with `--loadPath`:

```bash
./nemesisdb --config=default.json --loadName=mydata --loadPath=/some/other/path
```

<br/>

## Restore: Runtime
Data can be loaded at runtime with `SH_LOAD`:

```json
{
  "SH_LOAD":
  {
    "name":"mydata"
  }
}
```

This expects to find the data in the `session::save::path`.