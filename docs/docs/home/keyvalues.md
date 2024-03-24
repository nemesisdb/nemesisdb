---
sidebar_position: 10
displayed_sidebar: homeSidebar
---

# Key Values
NemesisDB uses the key-value nature of JSON to manage data.

If we have this JSON:

```json
{
  "age":50,
  "cars":["BMW", "Ford"],
  "address":
  {
    "city":"Paris",
    "country":"France"
  }
}
```

`age`, `cars` and `address` are keys:

- `age` has a number value
- `cars` has an array value 
- `address` has an object value


This is how NemesisDB treats the data when stored:

- `age` is mapped to `50`
- `cars` is mapped to `["BMW", "Ford"]`
- `address` is mapped to `{"city":"Paris","country":"France"}`


<!-- ![keyvalues](./img/kvset_kv.svg) -->

<br/>

## Set

To store the above, you can use the `KV_SET` command:

```json
{
  "KV_SET":
  {
    "keys":
    {
      "age":50,
      "cars":["BMW", "Ford"],
      "address":
      {
        "city":"Paris",
        "country":"France"
      }
    }
  }
}
```

- `keys` contains the keys and values to store

<br/>

## Get
To get the above keys:

```json
{
  "KV_GET":
  {
    "keys":["age", "cars", "address"]
  }
}
```

- `keys` an array of keys to retrieve

This returns:

```json
{
  "KV_GET_RSP":
  {
    "keys":
    {
      "age":50,
      "cars":["BMW", "Ford"],
      "address":
      {
        "city":"Paris",
        "country":"France"
      }
    }
  }
}
```

