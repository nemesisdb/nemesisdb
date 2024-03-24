---
sidebar_position: 10
---

# KV_SET
Stores key-values. If the key already exists, the value is overwritten.

- If you don't want to overwrite an existing key, use `KV_ADD`
- This command always returns a response. If you don't require a response you can use `KV_SETQ` which only sends a response on failure.


<br/>

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|unsigned int|Session token|Y|
|keys|object|Keys and values to store|Y|


<br/>

# `keys`
Contains the keys and their values:


```json title="Store three keys (forename, surname, email) with string values"
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
|st|uint|Status|
|keys|object|For each key that was set: `"<keyname>":<status>`|


Possible status values:

- Ok
- ParamMissing (`keys`)
- ValueTypeInvalid (`keys` not an object)

<br/>
<br/>

## Example

Set keys:
- user_1234_username a string
- user_1234_access an array
- user_1234_address an object

```json title="Set Request"
{
  "KV_SET":
  {
    "tkn":3442644399356403325,
    "keys":
    {
      "user_1234_name": "John Smith",
      "user_1234_access":["Secret Lab", "Helipad", "Server Room 1"],
      "user_1234_address":
      {
        "city":"London",
        "street":"Oxford Street"
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
    "st":1,
    "tkn": 3442644399356403325
  }
}
```

