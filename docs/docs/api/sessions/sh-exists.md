---
sidebar_position: 23
---

# SH_EXISTS
Checks if session token(s) exists.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkns|Array of unsigned int|Each unsigned int is a session token|Y|

<br/>

The response contains a bool flag for each token in the `tkns` array. If `true` it is valid to use with any command requiring a token.

<br/>


## Response

`SH_EXISTS_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|tkns|Object|Each token in the request is a key with a bool value|

<br/>

Possible status values:

- Ok
- CommandSyntax (`tkns` not present or not an array)

<br/>

```json title="Request: check if three sessions exist"
{
  "SH_EXISTS":
  {
    "tkns":
    [
      16145406615211810694,
      3573531683628185477,
      15472368655611898535
    ]
  }
}
```


```json title="Response: all but one exist"
{
  "SH_EXISTS_RSP":
  {
    "st": 1,
    "tkns":
    {
      "16145406615211810694": true,
      "15472368655611898535": true,
      "3573531683628185477": false
    }
  }
}
```