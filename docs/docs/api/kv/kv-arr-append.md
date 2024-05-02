---
sidebar_position: 58
---

# KV_ARR_APPEND
Appends items to an array. The array can be the key's value, or if the value is an object, the array can be at the object root.


:::info
This command does not check for duplicate items.
:::

<br/>


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|key|string|Key containing the value to update|Y|
|items|array|Items to append|Y|
|arrayName|string|If the key's value is an array, the `arrayName` must be omitted.<br/>If the key's value is an object, `arrayName` is the array's name.|N|

<br/>

:::info
The `tkn` must be set when the server mode is "kv_sessions".
:::


<br/>

## Response

`KV_ARR_APPEND_RSP`

|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|

Possible status values:

- Ok
- KeyNotExist
- Unknown (`key` or `arrayName` not an array)

See the [response status](./../Statuses) page for status values.



<br/>
<hr/>


## Examples

### Key is an array

```json
{
  "KV_SET":
  {
    "keys":
    {
      "user:1234:roles":
      [
        "Admin",
        "Reporter"
      ]
    }
  }
}
```

We can append two roles:

```json
{
  "KV_ARR_APPEND":
  {
    "key":"user:1234:roles",    
    "items":["Developer", "Tester"]
  }
}
```

### Key is an object

The key, `profile` is an object which contains an array `teams`:

```json
{
  "KV_SET":
  {
    "keys":
    {
      "profile":
      {
        "teams":["Warriors", "Legends"]        
      }
    }
  }
}
```

We can append `"Saracens"` to `teams`:

```json
{
  "KV_ARR_APPEND":
  {    
    "key":"profile",    
    "arrayName":"teams",
    "items":["Saracens"]
  }
}
```

We must set `arrayName` because the key (`profile`) is an object containing the `teams` array.