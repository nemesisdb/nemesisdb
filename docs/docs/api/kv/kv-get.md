---
sidebar_position: 50
---

# KV_GET
Get one or multiple keys.

The command requires the token and an array of keys to retrieve. 


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|keys|array|Array of keys to retrieve|Y|


## Response

`KV_GET_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|keys|object|The keys and values requested|Y|

<br/>

If a key does not exist it is not returned.

Possible status values:

- KeyNotExists
- ParamMissing (no `keys`)
- ValueTypeInvalid (`keys` not an array)

See the [response status](./../Statuses) page for status values.


## Examples

### Various Types

```json
{
  "KV_SET":
  {    
    "keys":
    {
      "user_1_name":"John",
      "user_1_dob":638236800000,
      "user_1_access":
      [
        {
          "location":"Secret Lab",
          "lastEntry":1711275303000
        },
        {
          "location":"Server Room",
          "lastEntry":1711034103000
        }
      ],
      "user_1_account":
      {
        "enabled":true,
        "creditBalance":34.50
      }
    }
  }
}
```

```json title="Request string and array keys"
{
  "KV_GET":
  {
    "keys":["user_1_name", "user_1_access"]
  }
}
```

```json title=Response
{
  "KV_GET_RSP":
  {
    "st": 1,
    "keys":
    {
      "user_1_name": "John",
      "user_1_access":
      [
        {
          "location": "Secret Lab",
          "lastEntry": 1711275303000
        },
        {
          "location": "Server Room",
          "lastEntry": 1711034103000
        }
      ]
    }
  }
}
```


### Key Does Not Exist

```json title="key user_1_dontexist does not exist"
{
  "KV_GET":
  {
    "keys":["user_1_name", "user_1_dontexist"]
  }
}
```

```json title="Only keys that exist are in the response"
{
  "KV_GET_RSP":
  {
    "keys":
    {
      "user_1_name": "John"
    }
  }
}
```