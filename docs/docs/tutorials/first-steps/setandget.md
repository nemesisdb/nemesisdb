---
sidebar_position: 3
displayed_sidebar: tutorialSidebar
---

# Set and Get

We'll store data of different types. With data types part of the JSON syntax, we don't have to care about types, only that the JSON is valid.


:::info Token
Remember to replace the `tkn` with your token.
:::


## String

Send:

```json
{
  "SH_SET":
  {
    "tkn":14247375118774737462,
    "keys":
    {
      "username":"Spongebob"
    }
  }
}
```
The stores the key `username` with string value "Spongebob".

- The `SH_SET_RSP` response contains `"username": 20` which is the key and status. This confirms "username" was successfully set
- If you send the command again, the status will change to `21`, which means an existing key has been overwritten (not an error)


## Array

Send:

```json
{
  "SH_SET":
  {
    "tkn":14247375118774737462,
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

This stores a key, `friends`, which is an array.


## Multiple
You can store multiple keys per request:

```json
{
  "SH_SET":
  {
    "tkn":14247375118774737462,
    "keys":
    {
      "email":"bob@thesea.com",
      "age":10
    }
  }
}
```

This stores two keys:

- `email` which is a string
- `age` which is a number


## Get Data

To get keys use `SH_GET`, which can get multiple keys in one request so we'll get all four:

```json
{
  "SH_GET":
  {
    "tkn":14247375118774737462,
    "keys":["username", "email", "age", "friends"]
  }
}
```

The response:

```json
{
  "SH_GET_RSP": {
    "tkn": 14247375118774737462,
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
  "SH_CLEAR":
  {
    "tkn":14247375118774737462
  }
}
```

The reponse contains `cnt`, confirming four keys were deleted.


## Object

We'll store the same data again but in an object:

```json
{
  "SH_SET":
  {
    "tkn":14247375118774737462,
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

This stores a key, `profile`, which is an object. 


## Get Object
Now when we want the user's profile data, we just have to get one key:

```json
{
  "SH_GET":
  {
    "tkn":14247375118774737462,
    "keys":["profile"]
  }
}
```


## The End
That's the basics of setting and getting.

This guide shows one session for one user. This pattern works for many users, each with their own session. Clients just need to use the appropriate token to switch users.

:::info
- `SH_SET` overwrites the key if it already exists. If you don't want this, you can use `SH_ADD` which won't overwrite
:::