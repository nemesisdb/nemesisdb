---
sidebar_position: 3
displayed_sidebar: tutorialSidebar
---

# Set and Get

We'll store data of different types. With data types part of the JSON syntax, we don't have to care about types, only that the JSON is valid.

To interact with data, the Key Value API is used. Its commands begin `KV_`.


## String

Send:

```json
{
  "KV_SET":
  {
    "keys":
    {
      "username":"Spongebob"
    }
  }
}
```
The stores the key `username` with string value "Spongebob".

- The `KV_SET_RSP` response contains `"username": 20` which is the key and status. This confirms "username" was successfully set
- If you send the command again, the status will change to `21`, which means an existing key has been overwritten (not an error)


## Array

Send:

```json
{
  "KV_SET":
  {
    "keys":
    {
      "friends":["Crabs","Patrick"]
    }
  }
}
```

This stores a key, `friends`, which is an array.


## Multiple
You can store multiple keys per request:

```json
{
  "KV_SET":
  {
    "keys":
    {
      "email":"bob@thesea.com",
      "age":10
    }
  }
}
```


## Get Data

To get keys use `KV_GET`, which can get multiple keys in a single request:

```json
{
  "KV_GET":
  {
    "keys":["username", "email", "age", "friends"]
  }
}
```

The response:

```json
{
  "KV_GET_RSP":
  {
    "keys":
    {
      "username": "Spongebob",
      "email": "bob@thesea.com",
      "age": 10,
      "friends":
      [
        "Crabs",
        "Patrick"
      ]
    }
  }
}
```

But what if we want all this in a single key? We can use an object.


## Clear Keys
Start by clearing all keys:

```json
{
  "KV_CLEAR":{}
}
```

The reponse contains `cnt`, confirming four keys were deleted.


## Object

Store the same data again but in an object:

```json
{
  "KV_SET":
  {
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
When we want the user's profile data, we just have to get one key:

```json
{
  "KV_GET":
  {
    "keys":["profile"]
  }
}
```


## Conclusion
That's the basics of setting and getting, other common commands are:

- `KV_ADD` : does not overwrite the key if it already exists
- `KV_COUNT` : Return the number of keys present 
- `KV_RMV` : delete key(s)
- `KV_CLEAR_SET` : clears all keys and sets new keys in a single command

<br/>


:::note
It is not possible to get keys based on a pattern, the exact key name is required.
:::
