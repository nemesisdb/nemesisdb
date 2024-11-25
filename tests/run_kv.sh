#!/bin/bash

if pgrep -x "nemesisdb" > /dev/null
then
  echo "FAIL: server already running"
else
  
  # to find base.py
  BASE=$(pwd)
  # to find Py API
  PY_API=$(pwd)/../apis/python
  
  export PYTHONPATH="$BASE:$PY_API"

  source ./useful.sh  

  run_kv_server
  
  if [ "$1" = "skip" ]; then
    export NDB_SKIP_SAVELOAD=1
  fi

  
  cd kv > /dev/null
  python3 -m unittest -f
  cd - > /dev/null

  kill_server
  
fi
