#!/bin/sh

if [ -z "$1" ]
then
  echo "Error: no version is set"
  exit 1;
fi

docker build --target publish -f docker/Dockerfile.alpine -t nemesisdb/nemesisdb:$1 .

docker push nemesisdb/nemesisdb:$1

docker tag nemesisdb/nemesisdb:$1 nemesisdb/nemesisdb:latest

docker push nemesisdb/nemesisdb:latest


# Linux run: 
#   docker run --rm -d --network host --name test1 nemesisdb/nemesisdb:0.3.2
# 
# but first change Docker context from the Docker Desktop with:
#
#   docker context use default
#
# Docker Desktop uses a VM, so screws the --network host
#    
