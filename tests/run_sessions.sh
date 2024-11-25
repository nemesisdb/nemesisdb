#!/bin/bash

if pgrep -x "nemesisdb" > /dev/null
then
  echo "FAIL: server already running"
else
  
  # to find base.py
  export PYTHONPATH=$(pwd)
  
  source ./useful.sh  

  run_session_server
  
  if [ "$1" = "skip" ]; then
    export NDB_SKIP_SAVELOAD=1
  fi

  
  cd sessions > /dev/null
  python3 -m unittest -f
  cd - > /dev/null

  kill_server
  
fi
