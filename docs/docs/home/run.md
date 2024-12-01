---
sidebar_position: 7
displayed_sidebar: homeSidebar
---

# Run

:::info
These instruction are for the Debian package. See [here](/category/docker) for Docker instructions.
:::

The bulk of the server config are set in a JSON [config](./config) file.

The server includes a default config which starts the server on `127.0.0.1:1987`.


## Arguments

The command line arguments are preceeded with `--`:

|Argument|Required|Description|
|:---|:---:|:---|
|config|Y|Path to the config file|
|loadName|N|The name of a save point containing data to restore. The name would have been used with `SH_SAVE`|
|loadPath|N|Path to directory containing the `loadName` data. If not set, uses the `persist::path` in the config file|

<br/>

:::note
- When persist is enabled the `path` must exist and be a directory
- You can disable save but still restore data
:::


<br/>

See [Persist Data](./persist) for more information on restoring.


## Examples

<br/>

### Restore

``` title="Start and restore, using path in config"
./nemesisdb --config=default.jsonc --loadName=10k_10000

NemesisDB v0.3.5 starting
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

``` title="Start and restore with alternative path"
./nemesisdb --config=default.jsonc --loadName=mydata --loadPath=/some/other/path

NemesisDB v0.3.5 starting
Registering signals
Reading config
Load Path: "/some/other/path"
Load Name: mydata
Reading metadata in "/some/other/path/mydata/1700433845873084066/md"
Loading from "/some/other/path/data/mydata/1700433845873084066/data"
-- Load --
Status: Success
Sessions: 10000
Keys: 50000
Time: 94ms
----------
Ready
```

<br/>


``` title="Load name exists but contains no data (not an error)"
./nemesisdb --config=default.jsonc --loadName=empty

NemesisDB v0.3.7 starting
Registering signals
Reading config
Load Path: "./data"
Load Name: empty
Reading metadata in "./data/empty/1701179912205808300/md"
Loading from "./data/empty/1701179912205808300/data"
-- Load --
Status: Success 
Sessions: 0
Keys: 0
Time: 0
----------
Ready
```

