---
slug: /
---

# NemesisDB

NemesisDB is a session based JSON store. At the moment it is in-memory storage only, a future release will include saving data to disk.

:::info 
NemesisDB is only available as a 64-bit Debian package for x86 CPUs (Intel/AMD).

A Docker image will be available soon.
:::



## Get Started
To get started you just need to:

1. Download the package
2. Install with `dpkg`
3. Run the server with the included default configuration


## Download
Download the latest version [here TODO](https://releases.nemesisdb.io).


## Install
Open a terminal, change to your download location and install with `dpkg`.

For example, to install version 0.3.0:

```bash
sudo dpkg -i nemesisdb_0.3.0_amd64.deb
```
<br/>

This installs the server and default config file to `/usr/local/bin/nemesisdb`.


## Run
To start the server:

```bash
cd /usr/local/bin/nemesisdb
./nemesisdb --config=default.json
```

This starts the server listening on `127.0.0.1:1987` for WebSocket clients.

<br/>

# Next Steps
A good place to start is to understand the basics of [sessions](/tutorials/sessions/what-is-a-session).
