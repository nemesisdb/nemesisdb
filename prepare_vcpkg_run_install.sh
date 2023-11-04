#!/bin/sh

cd vcpkg

echo
echo WARNING doing this in VS Code terminal may b0rk. Run in raw terminal.
echo

echo 
echo Bootstrap ...
echo 

./bootstrap-vcpkg.sh

echo 
echo Installing ...
echo 

./vcpkg install nlohmann-json boost-fiber[numa] boost-program-options boost-beast gtest uwebsockets --clean-after-build