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

  # kv
  run_kv_server
    
  cd sv > /dev/null
  python3 -m unittest -f test_server_info
  cd - > /dev/null

  kill_server
  
fi
