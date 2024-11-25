#!/bin/bash


function run_server()
{
  cd ../server/Release/bin/ > /dev/null
  ./nemesisdb $1 > /dev/null &
  cd - > /dev/null
  
  while ! pgrep -f "nemesisdb" > /dev/null; do
    sleep 0.5
  done
}


function run_kv_server()
{
  run_server "--config=$(pwd)/server_kv.jsonc"
}


function run_session_server()
{
  run_server "--config=$(pwd)/server_sessions.jsonc"
}


function kill_server()
{
  pkill nemesisdb
  wait $!
}