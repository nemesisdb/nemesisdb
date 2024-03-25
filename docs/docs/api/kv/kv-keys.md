---
sidebar_position: 100
---

# KV_KEYS
Returns all the key names (values not included).

<br/>

## Response

`KV_KEYS_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|keys|string array|An array of key names|

If there are no keys, the `keys` array is empty.

Possible status values:

- Ok


<br/>

## Examples

### No Keys

```json
{
  "KV_KEYS":{}
}
```

Response:

```json
{
  "KV_KEYS_RSP":
  {
    "st": 1,
    "keys": []
  }
}
```


### Keys Exist

```json
{
  "KV_KEYS":{}
}
```

Response:

```json
{
  "KV_KEYS_RSP":
  {
    "st": 1,
    "keys": ["user_1_name","user_1_dob", "user_2_name", "user_2_dob"]
  }
}
```
