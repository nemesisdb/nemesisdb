---
sidebar_position: 2
displayed_sidebar: tutorialSidebar
---

# Configure

The settings for save and load are in the server config, which defaults to:

```json
"session":
{
  "save":
  {
    "enabled":false,
    "path":"./data"
  }
}
```

- When `enabled` is false:
  - `path` does not need to exist
  - `SH_SAVE` is disabled
  - `SH_LOAD` is still available
- When `enabled` is true:
  - `path` must exist and be a directory
  - The server will fail startup if `path` is not valid

<br/>

