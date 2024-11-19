---
sidebar_position: 6
displayed_sidebar: clientApisSidebar
sidebar_label: session_info_all
---

# session_info_all
Returns minimal information for all sessions.


## Returns


`tuple(bool, dict)`
- `bool` : `True` is the command was successful, otherwise `False`
- `dict` : information. See table below


|Param|Type|Meaning|
|:---|:---|:---|
|totalSessions|unsigned int|Number of sessions|
|totalKeys|unsigned int|Number of keys over all the sessions, i.e. if there are 10 sessions, each with 10 keys, `totalKeys` is 100.|