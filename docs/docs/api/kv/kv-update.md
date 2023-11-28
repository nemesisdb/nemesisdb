---
sidebar_position: 57
---

# KV_UPDATE
This updates a value inside an object or array, using a JSON path to identify the element to update.

This differs from `KV_SET` because `KV_SET` overwrites the entire key's value.

<br/>


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|tkn|unsigned int|Session token|Y|
|key|string|Key containing the value to update|Y|
|path|string|JSON path which returns an element to update|Y|
|value|various|This value will be assigned to the element returned by `path`|Y|


For example, if we store this:

```json
{
  "KV_SET":
  {
    "tkn":3241497441754231528,
    "keys":
    {
      "profile":
      {
        "address":
        {
          "city":"London"
        }
      }
    }
  }
}
```

We want to update the `city`. We could either overwrite the entire `profile` key with `KV_SET` or just update the `city` directly:

```json
{
  "KV_UPDATE":
  {
    "tkn":3241497441754231528,
    "key":"profile",
    "path":"$.address.city",
    "value":"Paris"
  }
}
```


<br/>

## Response

`KV_UPDATE_RSP`

|Param|Type|Meaning|
|:---|:---|:---|
|tkn|unsigned int|Session token|
|key|object|The same key as in the request|
|cnt|unsigned int|Number of values updated|


Possible status values:

- Ok
- KeyNotExist
- ParamMissing
- ValueTypeInvalid


See the [response status](./../Statuses) page for status values.



<br/>
<hr/>


## Examples

We are storing access permissions for a building. We use an `access` array to store information for each area:

```json
{
  "KV_SET":
  {
    "tkn":3241497441754231528,
    "keys":
    {
      "user":
      {
        "access":
        [
          {
            "name":"Super Secret Lab",
            "permit":true,
            "level":4
          },
          {
            "name":"Helipad",
            "permit":true,
            "level":3
          },
          {
            "name":"Normal Lab",
            "permit":true,
            "level":2
          },          
          {
            "name":"Cafe",
            "permit":true,
            "level":1
          }
        ]
      }
    }
  }
}
```

This user has level 4 access, but now they are downgraded to level 2, so we need to adjust these permissions.

We could use `KV_SET` to overwrite the entire `user` key but that would be wasteful for a larger object and requires us to have all the data.

So we use an update:


```json
{
  "KV_UPDATE":
  {
    "tkn":3241497441754231528,
    "key":"user",
    "path":"$.access[?(@.level > 2)].permit",
    "value":false
  }
}
```

- `tkn` the session token
- `key` the key containing the value to update
- `path` a JSON path to find the value we wish to update
- `value` the new value

The path says: "find all items in the `access` array with a level above 2 and return `permit`". 
 
The `value` is then assigned to the returned path (i.e. we set `permit` to false for areas with an access level > 2).