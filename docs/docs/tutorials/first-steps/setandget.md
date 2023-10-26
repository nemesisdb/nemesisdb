---
sidebar_position: 3
displayed_sidebar: tutorialSidebar
---

# Set and Get

We'll store data of different types. With data types part of the JSON syntax, we don't have to care about types, only that the JSON is valid.

To interact with data, the Key Value API is used. It's commands begin `KV_`.

:::info Token
Remember to replace the `tkn` with your token.
:::


## String

Send:

```json
{
  "KV_SET":
  {
    "tkn":"14247375118774737462",
    "keys":
    {
      "username":"Spongebob"
    }
  }
}
```

The response contains `"username": 20` which is the key and status. This confirms "username" was successfully set.

If you send the command again, the status will change to `21`, which means an existing key has been overwritten (not an error).


## Array

Send:

```json
{
  "KV_SET":
  {
    "tkn":"14247375118774737462",
    "keys":
    {
      "friends":
      [
        "Crabs",
        "Patrick"
      ]
    }
  }
}
```

This stores a key, "friends", which is an array.


## Multiple
You can store multiple keys per request:

```json
{
  "KV_SET":
  {
    "tkn":"14247375118774737462",
    "keys":
    {
      "email":"bob@thesea.com",
      "age":10
    }
  }
}
```

This stores two keys:

- "email" which is a string
- "age" which is a number


## Get Data

To get keys use `KV_GET`, which can get multiple keys in one request so we'll get all four:

```json
{
  "KV_GET":
  {
    "tkn":"14247375118774737462",
    "keys":["username", "email", "age", "friends"]
  }
}
```

The response:

```json
{
  "KV_GET_RSP": {
    "tkn": "14247375118774737462",
    "keys": {
      "username": "Spongebob",
      "email": "bob@thesea.com",
      "age": 10,
      "friends":[
        "Crabs",
        "Patrick"
      ]
    }
  }
}
```

This is okay, but what if we want all this in a single key? We can use an object.


## Clear Session Data
Let's clear our session data:

```json
{
  "KV_CLEAR":
  {
    "tkn":"14247375118774737462"
  }
}
```

The reponse confirms that four keys have deleted (`cnt`).


## Object

We'll store the same data again but in an object:

```json
{
  "KV_SET":
  {
    "tkn":"14247375118774737462",
    "keys":
    {
      "profile":
      {
        "username": "Spongebob",
        "friends": ["Crabs","Patrick"],
        "email":"bob@thesea.com",
        "age":10
      }
    }
  }
}
```

This is storing a key "profile" which is an object. 


## Get Object
Now when we want the user's profile data, we just have to get one key:

```json
{
  "KV_GET":
  {
    "tkn":"14247375118774737462",
    "keys":["profile"]
  }
}
```


## The End
That's the basics of setting and getting, further info:

- `KV_SET` overwrites the key if it already exists. If you don't want this, you can use `KV_ADD` which won't overwrite
- `KV_SET` always returns a response. If you don't need confirmation, you can use `KV_SETQ` ("set quiet") which only responds on an error