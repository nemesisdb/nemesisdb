#!/bin/bash


# function run_server()
# {
#   cd ../server/Release/bin/ > /dev/null
#   ./nemesisdb $1 > /dev/null &
#   cd - > /dev/null
  
#   while ! pgrep -f "nemesisdb" > /dev/null; do
#     sleep 0.5
#   done
# }


function run_server()
{
  CONFIG=$(pwd)/server.jsonc

  cd ../server/Release/bin/ > /dev/null
  ./nemesisdb "--config=${CONFIG}" > /dev/null &
  cd - > /dev/null
  
  while ! pgrep -f "nemesisdb" > /dev/null; do
    sleep 0.5
  done
}


function kill_server()
{
  pkill nemesisdb
  wait $!
}