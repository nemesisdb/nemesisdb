---
sidebar_position: 1
---

# KV_SET
Stores data. This command always returns a response. If you don't require a response you can use `KV_SETQ` which sends a response on failure.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|string|Session token|Y|
|keys|object|Keys and values to store|Y|


<br/>

# `keys`

Each key and its value are stored in the session. A value can be:

- string
- number
- bool
- object
- array


This stores three keys, "forename", "surname" and "email" with their respective string values:

```json
{
  "KV_SET":
  {
    "tkn":"12345678",
    "keys":
    {
      "forename":"James",
      "surname":"Bond",
      "email":"jb@mi5.com"
    }
  }
}
```

The above can be also stored as an object with key "profile":

```json
{
  "KV_SET":
  {
    "tkn":"12345678",
    "keys":
    {
      "profile":
      {
        "forename":"James",
        "surname":"Bond",
        "email":"jb@mi5.com"
      }      
    }
  }
}
```



<br/>


## Response

`KV_SET_RSP`

See the [response status](./../Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|string|Session token|
|keys|object|For each key that was set: `"<keyname>":<status>`|


Possible status values:

- KeySet
- ParamMissing (`keys`)
- ValueTypeInvalid (`keys` not an object)

<br/>

## Examples

### Store One Key

```json
{
  "KV_SET":
  {
    "tkn":"3442644399356403325",
    "keys":
    {
      "user":
      {
        "username":"Potato",
        "email":"spud@email.com",
        "avatar":"path/to/img.png"
      }
    }
  }
}
```

Response:

```json
{
  "KV_SET_RSP":
  {
    "tkn": "3442644399356403325",
    "keys":
    {
      "user": 20
    }
  }
}
```

This means key "user" was set (`20`). If "user" key already existed, the status would be `21` to show its value has been replaced.

<br/>

### Store Multiple Keys

```json
{
  "KV_SET":
  {
    "tkn":"3442644399356403325",
    "keys":
    {
      "stats":
      {
        "visits":45642,
        "averageSession":300
      },
      "ui":
      {
        "theme":"dark",
        "layout":"stacked"
      }
    }
  }
}
```


Response:

```json
{
  "KV_SET_RSP": {
    "tkn": "3442644399356403325",
    "keys": {
      "stats": 20,
      "ui": 20
    }
  }
}
```
