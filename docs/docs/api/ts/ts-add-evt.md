---
sidebar_position: 10
---

# TS_ADD_EVT
Add event(s) to a time series.


|Param|Type|Description|
|:---|:---:|:---|
|ts|string|Name for the series|
|t|array  of times|An array of timestamps (64-bit integers)|
|evt|array of objects|Each object represents an event|

<br/>

Existing data in the time series is indexed before the response is returned. Thereafter it is updated on each call to `TS_ADD_EVT`.


## Indexes
Only top level event members can be indexed. For example, a time series which stores events for temperature and pressure sensor readings, could look like:

```json
{
  "temperature":5.5,
  "pressure":
  {
    "value":23
  }
}
```

We can index `temperature` but not `value` because it is not a top level member. We must move `pressure` to the top level:


```json
{
  "temperature":5.5,
  "pressure": 23
}
```

Now we can index both `temperature` and `pressure`.


With these members indexed they can be used in `TS_GET`:

```json
{
  "TS_GET":
  {
    "ts":"sensors",
    "rng":[1,3000],
    "where":
    {
      "temperature":
      {
        ">":3.0
      },
      "pressure":
      {
        "[]":[15,25]
      }
    }
  }
}
```

- This says, "Get the events which occured between times 1 and 3000 inclusive, where the temperature is greater then 3 and the pressure is between 15 and 25 inclusive.


## Examples

- Add three events at times 100, 102 and 105:

```json
{
  "TS_ADD_EVT":
  {
    "ts":"user_login",
    "t":[100, 102, 105],
    "evt":
    [
      {"userId":"1000", "action":"login"},
      {"userId":"1001", "action":"login"},
      {"userId":"1002", "action":"logout"}
    ]
  }
}
```

- You can have multiple occurences of the same time:

```json
{
  "TS_ADD_EVT":
  {
    "ts":"user_login",
    "t":[110, 110, 112],
    "evt":
    [
      {"userId":"1000", "action":"logout"},
      {"userId":"1001", "action":"logout"},
      {"userId":"1003", "action":"login"}
    ]
  }
}
```


- Events can contain more than strings

```json
{
  "TS_ADD_EVT":
  {
    "ts":"log",
    "t":[500, 505],
    "evt":
    [
      {"program":"nemesisdb", "args":["--config=/etc/nemesisdb/config.json"], "user":"ndb", "root":false},
      {"program":"redis-server", "args":["/etc/redis/config.conf"], "user":"redis", "root":false}
    ]
  }
}
```



## Response

`TS_CREATE_INDEX_RSP`

See the [response status](./../TS-Statuses) page for status values.


|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|


Possible status values:

- Ok
- SeriesNotExist
- IndexExists

