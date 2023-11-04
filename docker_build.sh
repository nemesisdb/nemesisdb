#!/bin/sh

if [ -z "$1" ]
then
  echo "Error: no version is set"
  exit 1;
fi

docker build --target publish -f docker/Dockerfile.alpine -t nemesisdb/nemesisdb:$1 .

# NOTE after building, we want to run with: 
#   docker run --rm -d --network host --name test1 nemesisdb/nemesisdb:0.3.2
# 
# but we first need to change Docker context from the Docker Desktop because it uses a VM, so screws the --network host
#   docker context use default
