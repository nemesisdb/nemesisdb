---
sidebar_position: 55
---

# KV_FIND
Searches the values and returns keys, data or paths.


|Param|Type|Meaning|Required|
|:---|:---|:---|:---:|
|rsp|string|Must be one of: `keys`, `kv`, or `paths`|Y|
|path|string|A JSON Path applied to each key's value|Y|
|keys|array|An array of keys. If present, only these keys are searched|N|

<br/>

:::info
The `tkn` must be set when the server mode is "kv_sessions".
:::

<br/>

The response depends on `rsp` ([examples](#examples) below):


## `"keys"`
Only keys matching the path are returned.


## `"paths"`
The paths to the element(s) matching the path returned. The key in which the values are found are not returned.


## `"kv"`
The keys and values are returned for matching paths.


## Response
`KV_FIND_RSP`

The `tkn` is always returned so this is omitted from the tables below.

<br/>

### keys

|Param|Type|Meaning|
|:---|:---|:---|
|keys|array|Array of keys|

<br/>

### paths

|Param|Type|Meaning|
|:---|:---|:---|
|paths|array|Array of paths|

<br/>

### kv

|Param|Type|Meaning|
|:---|:---|:---|
|keys|object|For each key that matches the criteria there's an entry:  `"<keyname>":<value>`|

<br/>

## Examples

These examples are based on storing this:

```json
{
  "KV_SET":
  {
    "keys":
    {
      "Devices":
      [
        {
          "make":"Apple",
          "model":"iPhone 14",
          "sensors":
          [
            {"type":"Temperature", "status":"Healthy", "id":"temp1"},
            {"type":"Accelerometer", "status":"Healthy", "id":"accel1"}
          ],
          "readings":
          {
            "accel1":
            [
              {"value":2.5, "time":12345600},
              {"value":1.565, "time":12345601}
            ],
            "temp1":
            [
              {"value":12.5, "time":12345603},
              {"value":12.8, "time":12345606}
            ]
          }
        },
        {
          "make":"Samsung",
          "model":"S23",
          "sensors":
          [
            {"type":"Temperature", "status":"Healthy", "id":"temp1"},
            {"type":"Accelerometer", "status":"Healthy", "id":"accel1"},
            {"type":"ThingyMaBob", "status":"Disabled", "id":"thingy1"}
          ],
          "readings":
          {
            "temp1":
            [
              {"value":24.0, "time":12345603},
              {"value":25.5, "time":12345604}
            ]
          }
        } 
      ]
    }
  }
}
```

- A "Device" key which is an array of objects
- Each object represents some device info (sensors and readings)


### Keys

```json title="Find if we have a Samsung device"
{
  "KV_FIND":
  {
    "rsp":"keys",
    "path":"$[?(@.make == 'Samsung')]"
  }
}
```

Response:

```json
{
  "KV_FIND_RSP":
  {
    "keys": ["Devices"]
  }
}
```
<hr/>

```json title="Find if we have an Apple iPhone 14"
{
  "KV_FIND":
  {
    "rsp":"keys",
    "path":"$[?(@.make == 'Apple' && @.model == 'iPhone 14')]"
  }
}
```

Response:

```json
{
  "KV_FIND_RSP":
  {
    "keys": ["Devices"]
  }
}
```

<hr/>

```json title="Find if we have an Apple iPhone 11"
{
  "KV_FIND":
  {
    "rsp":"keys",
    "path":"$[?(@.make == 'Apple' && @.model == 'iPhone 11')]"
  }
}
```

Response:

```json title="Keys is empty, no iPhone 11"
{
  "KV_FIND_RSP":
  {
    "keys": []
  }
}
```


### Paths

Only the path is returned

```json title="Find devices that have at least three sensors"
{
  "KV_FIND":
  {
    "rsp":"paths",
    "path":"$[?(@.sensors.length >= 3)]"
  }
}
```

Response:

```json title="iPhone 14"
{
  "KV_FIND_RSP":
  {
    "paths": ["$[1]"]
  }
}
```


### KV
For this we'll use this data:

```json
{
  "KV_SET":
  {
    "keys":
    {
      "loginsValid":
      [
        {"timestamp":1234, "location":"London"},
        {"timestamp":1235, "location":"London"},
        {"timestamp":1236, "location":"Paris"},
        {"timestamp":1238, "location":"London"}
      ],
      "loginsFailed":
      [
        {"timestamp":1235, "location":"New York"},
        {"timestamp":1235, "location":"London"},
        {"timestamp":1237, "location":"New York"}
      ]
    }
  }
}
```

Get the valid logins if any are between timestamps 1234 and 1238:

```json
{
  "KV_FIND":
  {
    "rsp":"kv",
    "keys":["loginsValid"],
    "path":"$[?(@.timestamp > 1234 && @.timestamp < 1238)]"
  }
}
```

Response:

```json
{
  "KV_FIND_RSP":
  {
    "kv": {
      "loginsValid": [
        {
          "timestamp": 1234,
          "location": "London"
        },
        {
          "timestamp": 1235,
          "location": "London"
        },
        {
          "timestamp": 1236,
          "location": "Paris"
        },
        {
          "timestamp": 1238,
          "location": "London"
        }
      ]
    }
  }
}
```


Get the failed logins if any are from London:

```json title="Restrict to loginsFailed key"
{
  "KV_FIND":
  {
    "rsp":"kv",
    "keys":["loginsFailed"],
    "path":"$[?(@.location == 'London')]"
  }
}
```

Response:

```json
{
  "KV_FIND_RSP":
  {
    "kv": {
      "loginsFailed": [
        {
          "timestamp": 1235,
          "location": "New York"
        },
        {
          "timestamp": 1235,
          "location": "London"
        },
        {
          "timestamp": 1237,
          "location": "New York"
        }
      ]
    }
  }
}
```