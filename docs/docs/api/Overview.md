---
sidebar_position: 1
displayed_sidebar: apiSidebar
---

# Overview

There are seperate APIs for time series and key-values (which also requires the session's API).


- All commands must be in upper case
- All parameters are in lower case unless stated otherwise
- All commands are a JSON object
- Commands which return a response contain a status (`st`) which is an unsigned integer

There is a TLDR for [time series](../home/tldr-ts) and [key value](../home/tldr-kv).
<br/>

## Time Series
These are to create and delete time series, add events and search for events.

Use [`TS_CREATE`](ts/ts-create-series) after which you add events with [`TS_ADD_EVT`](ts/ts-add-evt) and get/search events with [`TS_GET`](ts/ts-get).

To use `where` in `TS_GET` or `TS_GET_MULTI` you must index event members with [`TS_CREATE_INDEX`](ts/ts-create-index).

<br/>

## Key Value
These store, update, find and retrieve session data. They start `KV_`.

The commands require a session token which is returned by `SH_NEW` and `SH_OPEN`.

A good place to start is [First Steps](../tutorials/first-steps/setup) which shows how to create a session and store/get data.

### Sessions
Beginning with `SH_`, these commands create, open, end and return session information. 

- Each session has a dedicated map (rather than one large map for all keys)
- Keys belong to a session map 
- A session does not belong to a particular client and a client can create multiple sessions
- Switch session by changing the session token (`tkn`)

