---
sidebar_position: 7
displayed_sidebar: clientApisSidebar
sidebar_label: session_save
---

# session_save
Save one or multiple sessions.

The saved sessions can be restored with [session_load()](./Load).


|Param|Type|Description|Required|
|--|:-:|--|:-:|
|name|str|The name of the dataset|Y|
|tkns|List[int]|A list of tokens to persist. Default is empty to persist all tokens|N|


The `name` is used with `session_load()` to restore at runtime or with `--loadName` to load at startup.

:::note
- Persistance must be enabled in the server config
:::


## Returns
None


## Examples

```py title='Save all'
client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.create_session()
if session.isValid:
  await client.set({'fname':'james', 'sname':'smith'}, session.tkn)
  await client.session_save('my_data')
```

<br/>

```py title='Save using token list'
dataSetName = 'my_data'
nSessions = 10

client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

# create sessions
tokensToSave = list()
for s in range(0, nSessions):
  session = await client.create_session()
  tokensToSave.append(session.tkn)
  await client.set({f'session{s}_key':'some_value'}, session.tkn)


print(tokensToSave)
await client.save(dataSetName, tokensToSave)

# clear and then load
await client.end_all_sessions()

rsp = await client.load(dataSetName)
print(rsp)
```