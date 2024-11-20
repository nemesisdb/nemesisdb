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


The `name` is used with `session_load()` to restore the same sessions.

:::note
This command is only for when sessions are enabled. If disabled, use [save()](../Save).
:::


## Returns

`bool` : `True` is the command was successful, otherwise `False`


## Examples

```py
client = SessionClient()
await client.open('ws://127.0.0.1:1987/')

session = await client.create_session()
if session.isValid:
  await client.set({'fname':'james', 'sname':'smith'}, session.tkn)
  await client.session_save('my_data', [session.tkn])
```