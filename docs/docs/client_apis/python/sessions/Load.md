---
sidebar_position: 8
displayed_sidebar: clientApisSidebar
sidebar_label: session_load
---

# session_load
Restore sessions that were saved with [session_save()](./Save).


|Param|Type|Description|Required|
|--|:-:|--|:-:|
|name|str|The name of the dataset|Y|



:::note
- Persistance does _not_ have to be enabled for this command.
- This command is only for when sessions are enabled. If disabled, use [load()](../Load).
:::


## Returns
`dict` : See table below.



|Param|Type|Meaning|
|:---|:---|:---|
|sessions|unsigned int|Number of sessions loaded|
|keys|unsigned int|Number of keys loaded|



## Examples

```py title='Save and load one session'
client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

# clear before start
(cleared, count) = await client.end_all_sessions()
assert cleared

session = await client.create_session()
if session.isValid:
  dataSetName = 'my_data'
  
  await client.set({'fname':'james', 'sname':'smith'}, session.tkn)
  
  # save to filesystem
  await client.session_save(dataSetName, [session.tkn])

  # clear and restore
  (cleared, count) = await client.end_all_sessions()
  assert cleared and count == 1

  # restore keys
  (loaded, rsp) = await client.session_load(dataSetName)
  assert loaded and rsp['sessions'] == 1 and rsp['keys'] == 2
```
