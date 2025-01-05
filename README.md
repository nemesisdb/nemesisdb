> [!WARNING]
> This repo is no longer maintained. I've started a new data cache which uses FlatBuffers:  [fcache](https://github.com/ccooper1982/fcache).

<br/>

# NemesisDB
NemesisDB is an in-memory database using JSON over websockets. There are APIs for:

- Key values
- Arrays
- Lists

<br/>

## KV
- A JSON key is mapped to its value
- Keys cannot expire, they must be deleted by command
- A future update will add key expiry

## Arrays
These are fixed sized arrays, implemented in contigious memory, with versions for:

- Unsorted arrays for integers, strings and JSON objects
- Sorted arrays for integers and strings

## Lists
A node based linked list for JSON objects:

- Common features plus splicing
- More features will be added

<br/>
<br/>

> [!IMPORTANT]
> The Nemesis API docs have fallen behind after many recent changes, but the Python docs are current. 
> 

- Nemesis [docs](https://docs.nemesisdb.io/).
- Python API [docs](https://docs.nemesisdb.io/client_apis/Overview)


<br/>
<br/>



# Python API
There is an early version of a [Python API](https://github.com/nemesisdb/nemesisdb/tree/main/apis/python) with docs [here](https://docs.nemesisdb.io/client_apis/Overview).


KV:

```py
from ndb.client import NdbClient
from ndb.kv import KV

client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

kv = KV(client)
await kv.set({'username':'billy', 'password':'billy_password'})

username = await kv.get(key='username')
print (username) # billy

values = await kv.get(keys=('username','password'))
print (values) # {'password':'billy_password', 'username':'billy'}
```

Sorted integer array:

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
print(intersected) # [10,25,100]
```

Object list:

```py
from ndb.client import NdbClient
from ndb.lists import ObjLists

client = NdbClient()
await client.open('ws://127.0.0.1:1987/')

lists = ObjLists(client)

await lists.create('names')

# add 3 items
await lists.add('names', [{'name':'James'}, {'name':'Jane'}, {'name':'John'}])
# add new head (Jack, James, Jane, John)
await lists.add_head('names', {'name':'Jack'}) 
# overwrite Jane and John (Jack, James, Brian, Bryan)
await lists.set('names', [{'name':'Brian'},{'name':'Bryan'}], start=2)
# splice Brian and Bryan to a newly created list
await lists.splice(destName='other_names', srcName='names', srcStart=2, srcEnd=4)

print(await lists.get_rng('names', start=0))        # [{'name': 'Jack'}, {'name': 'James'}]
print(await lists.get_rng('other_names', start=0))  #[{'name': 'Brian'}, {'name': 'Bryan'}]
```

<br/>
<br/>


# Install
NemesisDB is available as a Debian package and Docker image:

- Package:  [Releases](https://github.com/nemesisdb/nemesisdb/releases) 
- Docker: [Docker Hub](https://hub.docker.com/r/nemesisdb/nemesisdb/tags)

You can compile for Linux, instructions below.


<br/>
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

After installing via dpkg:

- `cd /usr/local/bin/nemesisdb`
- Then `cd <version>`

After build:

- `cd <clone_location>/server/Release/bin`

<br/>

In both cases, start server listening on `127.0.0.1:1987` and persistence disabled:

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

