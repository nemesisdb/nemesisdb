---
sidebar_position: 2
displayed_sidebar: homeSidebar
---

# Debian Package

There is a [Docker](./docker/windows) image if you are running on Windows (note: WSL2 on Windows is not tested).

## Download

Download the latest version [here](https://releases.nemesisdb.io/package/nemesisdb_0.9_amd64.deb).


## Install
Open a terminal, change to the download location and install with `dpkg`.

The install location is: `/usr/local/bin/nemesisdb/<version>`.

For example, to install version 0.6.5:

```bash
sudo dpkg -i nemesisdb_0.6.5_amd64.deb
```
<br/>

This installs the server and default config file to `/usr/local/bin/nemesisdb/0.6.5`.


## Run
To start the server:

```bash
cd /usr/local/bin/nemesisdb/0.6.5
./nemesisdb --config=default.jsonc
```

This starts the server with persistance disabled and the WebSocket API listening on `127.0.0.1:1987`.