#!/bin/sh

if [ "$1" = "skip" ]; then
  export NDB_SKIP_SAVELOAD=1
fi

echo Run as: \"$0 skip\" to exclude save/load test

python3 -m unittest -f