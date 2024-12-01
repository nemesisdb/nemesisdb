---
sidebar_position: 120
displayed_sidebar: clientApisSidebar
sidebar_label: sh_load
---

# sh_load
Restore sessions that were saved with [session_save()](./Save).


|Param|Type|Description|Required|
|--|:-:|--|:-:|
|name|str|The name of the dataset|Y|


:::note
- The command is available even if persistance is disabled
:::


## Returns
`dict` : See table below.



|Param|Type|Meaning|
|:---|:---|:---|
|sessions|unsigned int|Number of sessions loaded|
|keys|unsigned int|Number of keys loaded|



## Examples

```py title='Save and load one session'
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create_session()
if session.isValid:
  dataSetName = 'my_data'
  
  await client.sh_set({'fname':'james', 'sname':'smith'}, session.tkn)
  
  # save to filesystem
  await client.sh_save(dataSetName, [session.tkn])

  # end all sessions
  await client.sh_end_all()
  
  # restore keys
  rsp = await client.sh_load(dataSetName)
  print(rsp)

  # get keys we loaded
  values = await client.sh_get(('fname', 'sname'), session.tkn)
  print(values)
```
