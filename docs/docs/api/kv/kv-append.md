---
sidebar_position: 90
---

# KV_APPEND
This appends a value to an existing object, array or string value.

|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|string|Session token|Y|


After the token the keys are listed with value(s) to be appended.


## Append to an object
Appends the members as a new members in the database. If appending to an object, the command value must also be an object.

Example: we have a `user` key which is an object and add `"age":35` to the object:

```json title="Add age to user object"
{
  "KV_APPEND":
  {
    "tkn":"1234",
    "user":
    {
      "age":35
    }
  }
}
```

## Append to an array
There are two ways to append to an array:


**Single Item**

To append a single item, the command does not need an array. 

`colours` is an array of strings, we can "Blue" like this:

```json title="Append single item to array"
{
  "KV_APPEND":
  {
    "tkn":"1234",
    "colors":"Blue"
  }
}
```
<br/>

**Multiple Items**

We have `devices` which is an array of objects and we add two new devices. We must use an array:

```json
{
  "KV_APPEND":
  {
    "tkn":"1234",
    "devices":
    [
      {"name":"Samsung", "model":"S23"},
      {"name":"Apple", "model":"iPhone 14"}
    ]
  }
}
```

## Append to a string
Appends a string to the existing string. The value in the command must be a string.

<br/>
<hr/>

## Example
For example, store some test data:


```json title="Set test data"
{
  "KV_SET":
  {
    "tkn":"5359008763088061727",
    "keys":
    {
      "user":
      {
        "username":"Potato",
        "email":"spud@email.com",
        "avatar":"path/to/img.png"
      },
      "access":
      [
        {"Reports":true},
        {"Admin":false}
      ],
      "browser":"Chrom",
      "colours":["r", "g"]
    }
  }
}
```

We want to:

- Add "forename" and "surname" to `user`
- Add a two object items to `access`
- Add string to `colors`
- Add an 'e' to `browser`


```json title="Append"
{
  "KV_APPEND":
  {
    "tkn":"5359008763088061727",
    "keys":
    {
      "user":
      {
        "forename":"James",
        "surname":"Smith"
      },
      "access":
      [
        {"Repo":true},
        {"Deploy":true}
      ],
      "colours":"b",
      "browser":"e"
    }
  }
}
```


## Response

`KV_APPEND_RSP`

|Param|Type|Meaning|
|:---|:---|:---|
|tkn|string|Session token|
|keys|object|Key names with status|

`keys` is an object with contents:

`"<keyname>":<status>`

Possible status values:

- Ok
- ValueTypeInvalid


### Example
The response to the `KV_APPEND` above is:

```json
{
  "KV_APPEND_RSP":
  {
    "tkn": "3961454475484276078",
    "keys":
    {
      "user": 1,
      "access": 1,
      "browser": 1
    }
  }
}
```


