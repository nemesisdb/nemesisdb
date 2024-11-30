---
sidebar_position: 1
---

# Session Keys
Keys are stored, retrieved, removed etc using `SH_` versions of the `KV_` command, for example `SH_SET`, `SH_GET` and `SH_RMV`.

They are the same, except the `SH_` version:

- Requires a token to identify the session
- Only apply to the session, i.e. `SH_CLEAR` only clears keys in the given session

To avoid duplication the session docs don't list each of these, instead refer to the `KV_` equivalent and set the `tkn` value.

```json title='Same as KV_SET but with a tkn'
{
  "SH_SET":
  {
    "tkn":260071646955705531,
    "keys":
    {
      "forename":"Billy",
      "surname":"Bob"
    }
  }
}
```