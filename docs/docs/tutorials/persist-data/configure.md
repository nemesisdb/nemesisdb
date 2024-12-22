---
sidebar_position: 2
displayed_sidebar: tutorialSidebar
---

# Configure

The settings for save and load are in the server [config](../../home/config), which defaults to:

```json
"persist":
{
  "enabled":false, 
  "path":"./data"
}
```


- When disabled:
  - `path` does not need to exist
  - `KV_SAVE` is disabled
  - `KV_LOAD` is available
  - `--loadName` and `--loadPath` at startup are available
- When enabled:
  - `path` must exist and be a directory
  - `KV_SAVE` is available


