---
sidebar_position: 120
displayed_sidebar: clientApisSidebar
sidebar_label: sh_load
---

# sh_load
Restore sessions that were saved with [sh_save()](./Save).


```py
sh_load(name: str) -> dict
```


|Param|Type|Description|Required|
|--|:-:|--|:-:|
|name|str|The name of the dataset, previously used with `sh_save()`|Y|


:::note
- The command is available even if persistance is disabled
:::


## Returns
|Key|Type|Meaning|
|:---|:---|:---|
|sessions|unsigned int|Number of sessions loaded|
|keys|unsigned int|Number of keys loaded|



## Examples

```py title='Save and load one session'
dataSetName = 'my_data'

client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()

await client.sh_set(session.tkn, {'fname':'james', 'sname':'smith'})

# save to filesystem
await client.sh_save(dataSetName, [session.tkn])
# delete all sessions
await client.sh_end_all()

# restore keys
rsp = await client.sh_load(dataSetName)
print(rsp) #  {'sessions':1, 'keys':2}

# get keys we loaded
values = await client.sh_get(session.tkn, keys=('fname', 'sname'))
print(values)
```
