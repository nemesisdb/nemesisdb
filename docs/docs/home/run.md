---
sidebar_position: 7
displayed_sidebar: homeSidebar
---

# Run
The bulk of the server config are set in a JSON [config](./config) file, which is in preference to endless command line arguments.

The server includes a default config which starts the server on `127.0.0.1:1987`. If this is unsuitable, change the `kv::ip` and `kv::port` as required.


## Arguments

The command line arguments are preceeded with `--`:

|Argument|Required|Description|
|:---|:---:|:---|
|config|Y|Path to the config file|
|loadName|N|The name of a save point containing data to restore. The name would have been used with `SH_SAVE`|

<br/>

:::note
- When save is enabled the `path` must exist and be a directory
- You can disable save but still restore data
:::

<br/>

See [Persist Data](./persist) for more information on restoring.


## Examples

### No Restore
```bash title="Successful start"
./nemesisdb --config=default.json

NemesisDB v0.3.4 starting
Registering signals
Reading config
Ready
```

<br/>

```bash title="Can't find config file"
./nemesisdb --config=dontexist.json

NemesisDB v0.3.4 starting
Registering signals
Reading config
Config file path not found
```

<br/>


```bash title="Save enabled but path does not exist"
./nemesisdb --config=default.json

NemesisDB v0.3.4 starting
Registering signals
Reading config
session::save::path is not a directory or does not exist
```


<br/>

### Restore

```bash title="Successful start and restore"
./nemesisdb --config=default.json --loadName=10k_10000

NemesisDB v0.3.4 starting
Registering signals
Reading config
Load Path: "./data"
Load Name: 10k_10000
Reading metadata in "./data/10k_10000/1700433845873084066/md"
Loading from "./data/10k_10000/1700433845873084066/data"
-- Load --
Status: Success
Sessions: 10000
Keys: 50000
Time: 94ms
----------
Ready
```

<br/>


```bash title="Load name does not exist"
./nemesisdb --config=default.json --loadName=dontexist

NemesisDB v0.3.4 starting
Registering signals
Reading config
Load Path: "./data"
Load Name: dontexist
Load name does not exist
```

<br/>


```bash title="Load name exists but contains no data"
./nemesisdb --config=default.json --loadName=emptydataset

NemesisDB v0.3.4 starting
Registering signals
Reading config
Load Path: "./data"
Load Name: dump1
No data
```

