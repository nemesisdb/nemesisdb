---
sidebar_position: 2
displayed_sidebar: homeSidebar
---

# Debian Package

This is most suitable if you are running on Linux or prefer to avoid Docker.

## Download

Download the latest version [here](https://releases.nemesisdb.io/package/nemesisdb_0.6_amd64.deb).


## Install
Open a terminal, change to the download location and install with `dpkg`.

The install location is: `/usr/local/bin/nemesisdb/<version>`.

For example, to install version 0.4:

```bash
sudo dpkg -i nemesisdb_0.4_amd64.deb
```
<br/>

This installs the server and default config file to `/usr/local/bin/nemesisdb/0.4`.


## Run
To start the server:

```bash
cd /usr/local/bin/nemesisdb/0.4
./nemesisdb --config=default.json
```

This starts the server in key-value mode with sessions disabled and the WebSocket API listening on `127.0.0.1:1987`.