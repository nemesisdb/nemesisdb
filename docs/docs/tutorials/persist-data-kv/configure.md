---
sidebar_position: 2
displayed_sidebar: tutorialSidebar
---

# Configure

The settings for save and load are in the server [config](../../home/config), which defaults to:

- enabled: `false`
- path: `"./data"`


- When `enabled` is false:
  - `path` does not need to exist
  - `KV_SAVE` is disabled
  - `KV_LOAD` is still available
- When `enabled` is true:
  - `path` must exist and be a directory
  - The server will fail startup if `path` is not valid

<br/>

