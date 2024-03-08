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

