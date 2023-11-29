---
sidebar_position: 2
displayed_sidebar: homeSidebar
---

# Debian Package

This is most suitable if you are running on Linux or prefer to avoid Docker.

## Download

Download the latest version [here](https://releases.nemesisdb.io/package/nemesisdb_0.3.7_amd64.deb).


## Install
Open a terminal, change to your download location and install with `dpkg`.

The install location is: `/usr/local/bin/nemesisdb/<version>`.

For example, to install version 0.3.7:

```bash
sudo dpkg -i nemesisdb_0.3.7_amd64.deb
```
<br/>

This installs the server and default config file to `/usr/local/bin/nemesisdb/0.3.7`.


## Run
To start the server:

```bash
cd /usr/local/bin/nemesisdb/0.3.7
./nemesisdb --config=default.json
```

This starts the server listening on `127.0.0.1:1987` for WebSocket clients.