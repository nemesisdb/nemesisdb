---
sidebar_position: 10
---

# KV_SET
Stores key-values. If the key already exists, the value is overwritten.

- If you don't want to overwrite an existing key, use `KV_ADD`


<br/>

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|keys|object|Keys and values to store|Y|

<br/>

# `keys`
Contains the keys and their values:


```json title="Store three keys (forename, surname, email) with string values"
{
  "KV_SET":
  {
    "keys":
    {
      "forename":"James",
      "surname":"Bond",
      "email":"jb@mi5.com"
    }
  }
}
```

Store a key with an object value. This sets a key 'profile':

```json title="Store object"
{
  "KV_SET":
  {
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

If we retrieved the 'profile' key the value returned is:

```json
{
  "forename":"James",
  "surname":"Bond",
  "email":"jb@mi5.com"
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
- ParamMissing 
- ValueTypeInvalid

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

```json title="Response"
{
  "KV_SET_RSP":
  {
    "st":1,
  }
}
```

