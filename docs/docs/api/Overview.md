---
sidebar_position: 1
displayed_sidebar: apiSidebar
---

# Overview

There are two APIs: Session and KeyValue.

Important points:

- All commands must be in upper case and all parameters are in lower case unless the docs say otherwise (i.e. `SH_NEW` has a `deleteSession` parameter).
- All commands are a JSON object, i.e. for `SH_NEW`:
```json
{
  "SH_NEW": {
    "name":"session1"
  }
}
```
- Many commands return a response that contains a status (`st`) which is an unsigned integer. Possible values are listed [here](./Statuses).

:::tip
Command names must be uppercase.

Parameters are case-sensitive.
:::

<br/>

## Session

Beginning with `SH_`, these commands create, open, end and return session information. 

A session does not belong to a particular client and a client can create multiple sessions, accessing their data as required by using the appropriate session token.

<br/>
<br/>


## KV

These store, update, find and retrieve session data. They start `KV_`.

The commands require a session token which is returned by `SH_NEW` and `SH_OPEN`.

<br/>
<br/>

# How to Use the APIs
A good place to start is the [First Steps](../tutorials/first-steps/setup) which shows how to create a session and store/get data.