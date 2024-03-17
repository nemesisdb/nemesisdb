---
sidebar_position: 23
---

# SH_EXISTS
Checks if session token(s) exists.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkns|Array of session tokens|Checks if each token exists|Y|

<br/>

Values in 'tkns' that are the incorrect type are ignored.

## Response

`SH_EXISTS_RSP`


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|exist|array|An array of session tokens that exist|
|notExist|array|An array of session tokens that do not exist|


<br/>

Possible status values:

- Ok
- ParamMissing
- CommandSyntax

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
    "exist": [16145406615211810694, 15472368655611898535],
    "notExist":[3573531683628185477]
  }
}
```