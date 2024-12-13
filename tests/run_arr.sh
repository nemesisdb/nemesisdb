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

  run_server
  
  if [ "$1" = "skip" ]; then
    export NDB_SKIP_SAVELOAD=1
  fi

  
  echo "Object Array"

  cd oarr > /dev/null
  python3 -m unittest -f
  cd - > /dev/null

  
  echo "Integer Array"
  cd iarr > /dev/null
  python3 -m unittest -f
  cd - > /dev/null


  echo "String Array"
  cd strarr > /dev/null
  python3 -m unittest -f
  cd - > /dev/null


  echo "Sorted Integer Array"
  cd sorted_iarr > /dev/null
  python3 -m unittest -f
  cd - > /dev/null


  echo "Sorted String Array"
  cd sorted_strarr > /dev/null
  python3 -m unittest -f
  cd - > /dev/null

  kill_server
  
fi
