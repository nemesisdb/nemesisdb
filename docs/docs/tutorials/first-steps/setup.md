---
sidebar_position: 1
displayed_sidebar: tutorialSidebar
---

# Setup

## Install Postman
The tutorial uses the Postman tool as a WebSocket client. The free version is suitable and can be [downloaded](https://www.postman.com/downloads/) for Windows, Linux and Mac.

## Configure and Start Server


1. Change to the install directory, default is `/usr/local/bin/nemesisdb`
2. In `default.jsonc` change `sessionsEnabled` to `true`.
3. In a terminal:

```bash title="Start server"
./nemesisdb --config=default.jsonc
```

This starts the server on `127.0.0.1:1987`.


## Check Connection

1. Open Postman
2. `File->New` or `ctrl+n` and select WebSocket
3. In "Enter URL" box, set `127.0.0.1:1987` and press Enter (or press Connect)

<br/>

![](img/postman_connected.png)

