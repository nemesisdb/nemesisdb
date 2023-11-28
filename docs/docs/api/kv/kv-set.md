---
sidebar_position: 10
---

# KV_SET
Stores key-values. If the key already exists it's value is overwritten.

- If you don't want to overwrite an existing key, use `KV_ADD`
- This command always returns a response. If you don't require a response you can use `KV_SETQ` which only sends a response on failure.



|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|unsigned int|Session token|Y|
|keys|object|Keys and values to store|Y|


<br/>

# `keys`


```json title="Store three keys (forename, surname, email)"
{
  "KV_SET":
  {
    "tkn":10838139838302738599,
    "keys":
    {
      "forename":"James",
      "surname":"Bond",
      "email":"jb@mi5.com"
    }
  }
}
```

```json title="The above can be also stored as an object in a single key (profile)"
{
  "KV_SET":
  {
    "tkn":10838139838302738599,
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
|tkn|uint|Session token|
|keys|object|For each key that was set: `"<keyname>":<status>`|


Possible status values:

- KeySet
- KeyUpdated (if key already existed)
- ParamMissing (`keys`)
- ValueTypeInvalid (`keys` not an object)

<br/>
<hr/>

## Examples

### Store One Key

```json
{
  "KV_SET":
  {
    "tkn":3442644399356403325,
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
    "tkn": 3442644399356403325,
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
    "tkn":3442644399356403325,
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
    "tkn": 3442644399356403325,
    "keys": {
      "stats": 20,
      "ui": 20
    }
  }
}
```
