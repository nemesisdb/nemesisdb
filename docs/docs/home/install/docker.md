---
sidebar_position: 1
displayed_sidebar: homeSidebar
---

# Docker

This is required if running on Windows unless you use WSL2 which is not supported.

The Docker image is based on Alpine Linux to keep deployment minimal.

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

# Run - Linux
On Linux, Docker containers can use the host's network which avoids having to map ports between host and container.

If you have Docker Desktop installed, you need to do this first:

```bash
docker context use default
```

<details>
  <summary>What does this do</summary>
  <div>
    <div>
    We need this because Docker Desktop on Linux runs within a VM, so the container will bind to the VM's virtual network rather than the host.
    Running this command tells Docker to use the 'default' context rather than the Docker Desktop, so the container can bind directly to the host's network ports.
    <br/><br/>
    See <a href="https://docs.docker.com/desktop/faqs/linuxfaqs/">here</a> for more.
    </div>
  </div>
</details>


## Start

The default config, included in the image, starts the server on `0.0.0.0:1987` so it is available from the host at `127.0.0.1:1987`.

### Default Config
Now we can start with:

```bash
docker run --rm -d --network host --name test1 nemesisdb/nemesisdb:latest
```

- `rm` deletes the container when you stop it. Remove `--rm` to keep it
- `d` runs detached to avoid blocking the terminal
- `network host` tells the container to use the host's network stack, which avoids having to map ports between host and container (i.e. `-p 1987:1987`)

Confirm the server is running and its ports are bound to the host:

```bash
netstat -tl | grep :1987
```

You should have at least one entry:
```bash
tcp        0      0 0.0.0.0:1987            0.0.0.0:*               LISTEN     
tcp        0      0 0.0.0.0:1987            0.0.0.0:*               LISTEN     
tcp        0      0 0.0.0.0:1987            0.0.0.0:*               LISTEN 
```


### Custom Config

If you want to bind the server to a different IP/port you can pass the path to a config file on the host.

To do this you need to mount a volume so the container can see the config file using the `-v` option.


```bash
docker run --rm -d -v ./server/configs:/configs --network host --name test1 nemesisdb/nemesisdb:latest --config=/configs/config.json
```

1. `-v` we want a volume mount
2. `./server/configs` is the host path containing the config file
3. `:/configs` sets the mount **within** the container

The final argument `--config=/configs/config.json` is the full path. We use `/configs/` because that's the mount point in the container (step 3)


:::tip
You could also use `-v ./server/configs:/configs:ro` to create a read-only mount.
:::


## Stop

```bash
docker stop test1
```

This will also delete the container because the container was started with `--rm`.

<br/>


# Run - Windows