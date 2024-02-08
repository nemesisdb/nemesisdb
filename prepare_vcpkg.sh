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

# note: boost-beast used in unit tests because uwebsockets does not have a client API
./vcpkg install nlohmann-json boost-fiber[numa] boost-program-options boost-beast gtest uwebsockets plog --clean-after-build