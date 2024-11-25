#!/bin/bash

if pgrep -x "nemesisdb" > /dev/null
then
  echo "FAIL: server already running"
else
  
  # to find base.py
  export PYTHONPATH=$(pwd)

  source ./useful.sh  

  # kv
  run_kv_server
    
  cd sv > /dev/null
  python3 -m unittest -f test_server_info_kv
  cd - > /dev/null

  kill_server
  

  # sessions
  run_session_server
    
  cd sv > /dev/null
  python3 -m unittest -f test_server_info_sessions
  cd - > /dev/null

  kill_server
fi
