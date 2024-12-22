---
sidebar_position: 1
displayed_sidebar: clientApisSidebar
---

# Overview


## Python
:::important
This API is new and likely to change. 
:::

Manage the connectiion:
 - `from ndb.client import NdbClient`

Each API has a separate class, i.e.:

- `from ndb.kv import KV` for key value
- `from ndb.lists import ObjLists` for object lists
- `from ndb.arrays import SortedIntArrays` for sorted integer arrays

All commands can raise a `ResponseError`:
  - The exception contains the status (`st`)
  - The exception contains `rsp` which is the full response

Some commands can raise a `TypeError` or `ValueError`
