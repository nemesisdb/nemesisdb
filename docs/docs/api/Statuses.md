---
sidebar_position: 2
displayed_sidebar: apiSidebar
---

# Status Values
All commands return a status (`st`), defined in `core/NemesisCommon.h`:


```cpp
enum class RequestStatus
{
  Ok                    = 1,
  OpCodeInvalid,
  JsonInvalid,
  PathInvalid,
  NoPath,
  CommandNotExist       = 10,
  CommandMultiple,
  CommandType,
  CommandSyntax,
  CommandDisabled,
  NotExist              = 22,
  ParamMissing          = 26,
  ValueMissing          = 40,
  ValueTypeInvalid,
  ValueSize,
  SaveComplete          = 120,
  SaveDirWriteFail,
  SaveError,
  Loading               = 140,
  LoadComplete,
  LoadError,
  Duplicate             = 160,
  Bounds                = 161,
  Unknown               = 1000
}
```