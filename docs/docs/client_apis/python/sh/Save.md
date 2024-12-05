---
sidebar_position: 110
displayed_sidebar: clientApisSidebar
sidebar_label: sh_save
---

# sh_save
Save one or multiple sessions.

The saved sessions can be restored with [sh_load()](./Load).


```py
sh_save(name: str, tkns = list()) -> None
```

|Param|Description|
|--|--|
|name|The name of the dataset.<br/>The `name` is used to load data at runtime with `kv_load()` or at startup.|
|tkns|Only persist these sessions. If empty, all sessions are persisted|


:::note
- Persistance must be enabled in the server config
- To persist sessions, use [sh_save()](../sh/Save)
:::




## Examples

```py title='Save all'
client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.sh_create()
if session.tknValid:
  await client.sh_set({'fname':'james', 'sname':'smith'}, session.tkn)
  await client.sh_save('my_data')
```

<br/>

```py title='Save using token list'
dataSetName = 'my_data'
nSessions = 10

client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

# create sessions, setting a key in each session
tokensToSave = list()
for s in range(0, nSessions):
  session = await client.sh_create()
  if session.tknValid:
    tokensToSave.append(session.tkn)
    await client.sh_set(session.tkn, {f'session{s}_key':'some_value'})


print(tokensToSave)
await client.sh_save(dataSetName, tokensToSave)

# clear and then load
await client.sh_end_all()

rsp = await client.sh_load(dataSetName)
print(rsp)
```