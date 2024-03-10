---
sidebar_position: 20
---

# TS_GET_MULTI
Search multiple time series, returning time and event values for each. It is an alternative to issuing multiple [`TS_GET`](./ts-get) commands for different series.

This is functionaly the same as `TS_GET` but the time series request and response terms are set separately to allow different terms for each.

The general form is:

```json
{
  "TS_GET_MULTI":
  {
    "seriesName1":
    {
      "rng":[min, max],
      "where":
      {
        // terms
      }
    },
    "seriesName2":
    {
      "rng":[min, max],
      "where":
      {
        // terms
      }
    },
    "seriesNameN":
    {
      "rng":[min, max],
      "where":
      {
        // terms
      }
    }
  }
}
```

Within each series:

|Param|Type|Description|
|:---|:---:|:---|
|rng|Array of **zero or two** times|Empty array means search all times, otherwise a min and max are provided. The min and max are inclusive|
|where|One or multiple object|**Optional** <br/>Each object key is the name of an event member which must be indexed, see below for details|

<br/>

:::note
This is the same as `TS_GET` but with `ts` omitted.
:::


## Examples

- Return alarms triggered and users who were granted access between times 100 to 1000 inclusive:

```json
{
  "TS_GET_MULTI":
  {
    "alarm_activity":
    {
      "rng":[100,1000],
      "where":
      {
        "triggered":
        {
          "==":true
        }
      }
    },
    "user_activity":
    {
      "rng":[100,1000],
      "where":
      {
        "action":
        {
          "==":"AccessGranted"
        }
      }
    }  
  }
}
```

<br/>


## Response

`TS_GET_MULTI_RSP`

See the [response status](./../TS-Statuses) page for status values.

An object per time series, with the key being the time series name. The contents is the same as `TS_GET_RSP` but without `ts`:

|Param|Type|Meaning|
|:---|:---|:---|
|st|unsigned int|Status|
|t|Array of times|The times meeting the criteria|
|evt|Array of objects|Each object is an event. The event at `evt[i]` occured at time `t[i]`|


Possible status values:

- Ok
- SeriesNotExist
- NotIndexed


For example, the response to the above request could be:

```json
{
  "TS_GET_MULTI_RSP":
  {
    "alarm_activity":
    {
      "st":1,
      "t":[105, 200],
      "evt":
      [
        {"triggered":true, "reason":"Opened"},
        {"triggered":true, "reason":"Thermal"}
      ]
    },
    "user_activity":
    {
      "st":1,
      "t":[100],
      "evt":
      [
        {"entrance":"Lab 12", "ident":"PIN", "user":"Dr Jones"}
      ]
    }  
  }
}
```