# Python API


[!WARNING]
This API is not production ready. I am not an experienced Python dev, so if you see improvements please do submit. This is a starting point for future work.


The API uses the [websockets](https://websockets.readthedocs.io/en/stable/) library which uses asyncio.


The full API is documentated on the NemesisDB [docs]() TODO.

Functions for all **common** commands are provided, except:

|Command|Reason|
|---|---|
|KV_SAVE|Coming soon|
|KV_LOAD|Coming soon|
|KV_UPDATE|Coming soon|
|KV_FIND|Coming soon|
|KV_ARR_APPEND|Coming soon|
|KV_SETQ|Not supported|
|KV_ADDQ|Not supported|

<br/>

# Quick Start

The `Client` class handles the connection and contains a function per NemesisDB command.


## Connect

```python
async def example_connect():
  client = Client("ws://127.0.0.1:1987/")
  listen_task = await client.listen()

  # call query functions

  await listen_task
```

- Open connection
- `client.listen()` returns the awaitable which runs until disconnection


## Set/Get Basics
Set then get `username` and `password` keys:

```python
async def example_setget():
  client = Client("ws://127.0.0.1:1987/")
  listen_task = await client.listen()

  setSuccess = await client.set({'username':'billy', 'password':'billy_passy'})

  if setSuccess:
    (getOk, values) = await client.get(['username'])
    if getOk:
      print(values)
    else:
      print('Query failed')

  await listen_task
```

Output:
```bash
{
  'username': 'billy'
}
```

- `set()` accepts a `dict` of key:value
- `get()` accepts an array of keys
  - Returns a tuple: `(bool, dict)`
  - `bool`: `True` if command was successful
  - `dict` : keys with values


Retrieve multiple keys:

```python
(getOk, values) = await client.get(['username', 'password'])
```

<br/>

## Set/Get Objects
A key's value can be an object, as here with `server_users`:

```json
{
  "server_ip":"123.456.7.8",
  "server_port":1987,
  "server_users":
  {
    "admins":["user1", "user2"],
    "banned":["user3"]
  }
}
```

Set the data then get `server_users`:

```python
async def example_setget_objects():
  client = Client("ws://127.0.0.1:1987/")
  listen_task = await client.listen()

  data = {  "server_ip":"123.456.7.8",
            "server_port":1987,
            "server_users":
            {
              "admins":["user1", "user2"],
              "banned":["user3"]
            }
          }

  setSuccess = await client.set(data)
  if setSuccess:
    (getOk, values) = await client.get(['server_users'])
    if getOk:
      print(values)
```

Output:

```bash
{
  'server_users': {'admins': ['user1', 'user2'], 'banned': ['user3']}
}
```