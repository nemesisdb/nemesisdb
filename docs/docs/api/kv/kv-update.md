---
sidebar_position: 110
---

# KV_UPDATE
This updates a value inside an object or array.

This is different to `KV_SET` because `KV_SET` would overwrite the entire object/array, whilst `KV_UPDATE` can update a value inside the object or a specific array item.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|string|Session token|Y|
|key|string|Key containing the object to select the `path`|
|`<path>`|various|JSON Pointer, the value will replace the value selected by the path (see [examples](#examples))|Y|

<br/>

:::note
JSON Pointer syntax requires an index when accessing arrays, you can't say "all", only a specific index.
:::

<br/>

## Response

`KV_UPDATE_RSP`

|Param|Type|Meaning|
|:---|:---|:---|
|tkn|string|Session token|
|key|object|Key names with status for each: `"<keyname>":<status>`|

Possible status values:

- Ok
- KeyNotExist
- ParamMissing (`key`)
- ValueTypeInvalid (`key` not a string)


See the [response status](./../Statuses) page for status values.



<br/>
<hr/>

## Examples

```json title="Test data"
{
  "KV_SET":
  {
    "tkn":"789794144062202656",
    "keys":
    {
      "user":
      {
        "username":"Potato",
        "email":"spud@email.com",
        "avatar":"img1.png"
      },
      "access":
      [
        {"Reports":true},
        {"Admin":false}
      ]
    }
  }
}
```

We want to update the "avatar" but if we used `KV_SET` we'd have to set the whole user object. But with `KV_UPDATE` we can just update avatar:

```json title="Update avatar image"
{
  "KV_UPDATE":
  {
    "tkn":"789794144062202656",
    "key":"user",
    "/avatar":"img2.png"
  }
}
```

Update an item in array using the index:

```json title="Update Admin access"
{
  "KV_UPDATE":
  {
    "tkn":"789794144062202656",
    "key":"access",
    "/1":{"Admin":true}
  }
}
```
<br/>

This is only to show how to update an array item. The `access` data would be tidier as an object:

```json
{
  "KV_SET":
  {
    "tkn":"789794144062202656",
    "keys":
    {      
      "access":
      {
        "Reports":true,
        "Admin":false
      }
    }
  }
}
```

And updated:

```json
{
  "KV_UPDATE":
  {
    "tkn":"789794144062202656",
    "key":"access",
    "/Admin":true
  }
}
```
