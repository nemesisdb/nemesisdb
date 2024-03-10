---
sidebar_position: 15
---

# TS_GET
Search a time series, returning time and event values.


|Param|Type|Description|
|:---|:---:|:---|
|ts|string|Name of the series|
|rng|array of **zero or two** times|Empty array means search all times, otherwise a min and max are provided. The min and max are inclusive|
|where|One or multiple object|**Optional** <br/>Each object key is the name of an event member which must be indexed, see below for details|

<br/>

`TS_GET` can be with or without `where`. Without `where`, all events are returned that meet the time `rng` criteria.

- Get everything in the whole time series

```json
{
  "TS_GET":
  {
    "ts":"sensors",
    "rng":[]
  }
}
```

- Get everything between times 1000 and 1500 inclusive:

```json
{
  "TS_GET":
  {
    "ts":"sensors",
    "rng":[1000,1500]
  }
}
```

<br/>

## Where Clause
The `where` clause defines conditions for event values:

- Each condition is a JSON object, with the key being the name of an indexed member (see [`TS_CREATE_INDEX`](./ts-create-index.md))
- The condition can contain one key which is an operator and its value (various JSON types)

For example, with a timeseries called "sensors" and events containing a "temperature" member, we can search the whole range for events with `temperature` greater than 3:

```json
{
  "TS_GET":
  {
    "ts":"sensors",
    "rng":[],
    "where":
    {
      "temperature":
      {
        ">":3.0
      }
    }
  }
}
```

Multiple conditions are possible and there's always a logical "and" relationship


```json
{
  "TS_GET":
  {
    "ts":"sensors",
    "rng":[],
    "where":
    {
      "temperature":
      {
        ">":3.0
      },
      "pressure":
      {
        "<":55
      }
    }
  }
}
```

This is `temperature > 3 && pressure < 55`.


### Operators
|Operator|Value Type|Description|
|:---|:---:|:---|
|>|Number or array|Greater than|
|>=|Number or array|Greater than or equal to|
|<|Number or array|Less than|
|<=|Number or array|Less than or equal to|
|==|Number, array, object, bool, string|Equal to|
|[]|Array of two numbers|Range operator: [min, max] inclusive|


## Examples

- Temperature range between 0 and 35.0 inclusive

```json
{
  "TS_GET":
  {
    "ts":"sensors",
    "rng":[],
    "where":
    {
      "temperature":
      {
        ">":[0.0,35.0]
      }
    }
  }
}
```


- A string array with specific values


```json
{
  "TS_GET":
  {
    "ts":"users",
    "rng":[],
    "where":
    {
      "devices_online":
      {
        "==":["192.168.0.10", "192.168.0.13", "192.168.0.12"]
      }
    }
  }
}
```


- A string array with specific values _but_ only in a specified time range (`rng`)


```json
{
  "TS_GET":
  {
    "ts":"users",
    "rng":[1000, 10000],
    "where":
    {
      "devices_online":
      {
        "==":["192.168.0.10", "192.168.0.13", "192.168.0.12"]
      }
    }
  }
}
```

<br/>


## Response

`TS_GET_RSP`

See the [response status](./../TS-Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|ts|string|Name of the series|
|st|unsigned int|Status|
|t|Array of times|The times meeting the criteria|
|evt|Array of objects|Each object is an event. The event at `evt[i]` occured at time `t[i]`|


Possible status values:

- Ok
- SeriesNotExist
- NotIndexed
