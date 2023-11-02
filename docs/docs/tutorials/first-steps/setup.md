---
sidebar_position: 1
displayed_sidebar: tutorialSidebar
---

# Setup

## Install Postman
The tutorial uses the Postman tool as a WebSocket client. The free version is suitable and can be [downloaded](https://www.postman.com/downloads/) for Windows, Linux and Mac.

## Start NemesisDB

We'll start the server with the default config.

1. Change to the install directory, default is `/usr/local/bin/nemesisdb`
2. In a terminal:

```bash title="Start server"
./nemesisdb --config=default.json
```

This starts the server on `127.0.0.1:1987`.


## Check Connection

1. Open Postman
2. `File->New` or `ctrl+n` and select WebSocket

![](img/postman_newwebsocket.png)


3. In "Enter URL" box, set `127.0.0.1:1987` and press Enter (or press Connect)

![](img/postman_connected.png)


We're ready to fire some commands at the server now.


:::info
The default view will have the "Response" pane below the request. If you prefer the two pane view as the screenshot, you do so by:

1. `File->Settings`
2. In the "General" tab, towards the right, select "Two-pane view"
:::
