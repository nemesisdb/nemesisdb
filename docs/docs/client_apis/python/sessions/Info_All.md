---
sidebar_position: 6
displayed_sidebar: clientApisSidebar
sidebar_label: sh_info_all
---

# sh_info_all
Returns minimal information for all sessions.


## Returns
`dict` : information. See table below


|Param|Type|Meaning|
|:---|:---|:---|
|totalSessions|unsigned int|Number of sessions|
|totalKeys|unsigned int|Number of keys over all the sessions, i.e. if there are 10 sessions, each with 10 keys, `totalKeys` is 100.|