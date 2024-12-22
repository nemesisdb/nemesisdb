# NemesisDB
NemesisDB is an in-memory keyvalue JSON database:

- All data and commands are JSON over WebSockets
- Keys and sessions can be saved to file
- Keys and sessions can be loaded at startup or runtime


API and further information in the [docs](https://docs.nemesisdb.io/).

Contents:
  - [Python API](#python-api)
  - [Design](#design)
  - [Install](#install)
  - [Build for Linux](#build---linux-only)
  

<br/>
<br/>



# Python API
There is an early version of a [Python API](https://github.com/nemesisdb/nemesisdb/tree/main/apis/python) with docs [here](https://docs.nemesisdb.io/client_apis/Overview).

Set then retrieve two keys, `username` and `password`:

```py
from ndb.client import NdbClient

client = NdbClient()

await client.open('ws://127.0.0.1:1987/')
await client.kv_set({'username':'billy', 'password':'billy_password'})

username = await client.kv_get(key='username')
print (username) # billy

values = await client.kv_get(keys=('username','password'))
print (values) # {'password':'billy_password', 'username':'billy'}
```

Arrays:

```py
from ndb.client import NdbClient
from ndb.arrays import SortedIntArrays


client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

sortedInts = SortedIntArrays(client)
await sortedInts.create('array1', capacity=5)
await sortedInts.create('array2', capacity=6)

await sortedInts.set_rng('array1', [25,10,50,100,80])
await sortedInts.set_rng('array2', [10,25,100,120,200,5])

intersected = await sortedInts.intersect('array1', 'array2')
print(intersected)
```

<br/>
<br/>


# Overview

The server uses a JSON API over websockets with APIs for:

- Key values
- Arrays
- Lists

<br/>

## KV
Keys are managed by the  `KV_` API.

- There is one map for all independent keys
- Keys cannot expire, they must be deleted by command. A future update will add key expiry

## Arrays
These are fixed sized arrays, implemented in contigious memory, with versions for:

- Unsorted arrays for integers, strings and JSON objects
- Sorted arrays for integers and strings

## Lists
A node based linked list for JSON objects.

<br/>
<br/>

# Install
NemesisDB is available as a Debian package and Docker image:

- Package:  [Releases](https://github.com/nemesisdb/nemesisdb/releases) 
- Docker: [Docker Hub](https://hub.docker.com/r/nemesisdb/nemesisdb/tags)

You can compile for Linux, instructions below.


<br/>
<br/>

# Design

As of version 0.5, the engine is single threaded. The instance is assigned to core 0 by default but can be configured in the server [config](https://docs.nemesisdb.io/home/config).

The multithreaded version is collecting GitHub dust on the [0.4.1](https://github.com/nemesisdb/nemesisdb/tree/0.4.1) branch.

<br/>

## Persist and Restore
Key values can be saved to file and restored at either runtime or at startup in the command line.


<br/>
<br/>

# Build - Linux Only

> [!IMPORTANT]
> C++20 required.

1. Clone via SSH with submodules: `git clone --recursive git@github.com:nemesisdb/nemesisdb.git`
2. Prepare and grab vcpkg libs: `cd nemesisdb && ./prepare_vcpkg.sh`
3. With VS Code (assuming you have C/C++ and CMake extensions):
    - `code .`
    - Select kit (tested with GCC 12.3 and GCC 13.1)
    - Select Release variant
    - Select target as nemesisdb
    - Build
4. Binary is in `server/Release/bin`

<br/>

## Run
Start listening on `127.0.0.1:1987` with persistence disabled:

`./nemesisdb --config=default.jsonc`

`ctrl+c` to exit.


<br/>
<br/>

# External Libraries
Externals are either GitHub submodules or managed by [vcpkg](https://vcpkg.io/en/).

Server:
- uWebsockets : WebSocket server
- jsoncons : json
- ankerl::unordered_dense for map and segmented map
- plog : logging
- uuid_v4 : create UUIDs with SIMD
- Boost Program Options : argv options

Tests Client (will be replaced with Python):
- nlohmann json
- Boost Beast (WebSocket client)
- Google test

