---
sidebar_position: 2
displayed_sidebar: homeSidebar
---


# Windows
Running NemesisDB with Docker is required on Windows. You can try the Debian package with WSL2 but this is not supported.


This is similar to Linux except:

- `--network host` is not available
- Read-only volume mount is not available


## Pull

```bash
docker pull nemesisdb/nemesisdb:latest
```

Confirm:

```bash
docker images
```

```bash
REPOSITORY            TAG       IMAGE ID       CREATED        SIZE
nemesisdb/nemesisdb   0.3.2     6c2973cf3e57   5 hours ago    17.9MB
```

## Start - Default Config
The default config, included in the image, starts the server on `0.0.0.0:1987` so it is available from the host at `127.0.0.1:1987`.



Start with:

```bash
docker run --rm -d -p 1987:1987  --name test1 nemesisdb/nemesisdb:latest
```


## Start - Custom Config
We need to create a network, mount a volume and pass the config file path:

Create a network on the `192.168.1.x` range:

```bash
docker network create --subnet 192.168.1.0/16 my-net
```

Start with:

```bash
docker run --rm -d --network=my-net --ip=192.1681.111 -v ./server/configs:/configs --name test1 nemesisdb/nemesisdb:latest --config=./configs/config.json
```

- The config file has `"kv" : {"ip":"192.168.1.111"}` set which is within the range of our `my-net` we just created
- `-v` mounts as `<hostpath>:<containerpath>` , so our host has a config file in `./server/configs`
- The path passed to the container is `./configs` because that's where it was mounted in the `-v`


## Stop

```bash
docker stop test1
```
