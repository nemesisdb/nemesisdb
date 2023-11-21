---
sidebar_position: 100
---

# KV_KEYS
Returns all the key names in the session. 

This only returns the key names, not the values.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|uint|Session token|Y|


<br/>

## Response

`KV_KEYS_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|tkn|uint|Session token|
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
  "KV_KEYS":
  {
    "tkn":13549976642705620562
  }
}
```

Response:

```json
{
  "KV_KEYS_RSP":
  {
    "tkn": 13549976642705620562,
    "st": 1,
    "keys": []
  }
}
```


### Keys Exist

```json
{
  "KV_KEYS":
  {
    "tkn":13549976642705620562
  }
}
```

Response:

```json
{
  "KV_KEYS_RSP":
  {
    "tkn": 13549976642705620562,
    "st": 1,
    "keys": ["profile","stats"]
  }
}
```
