#!/bin/sh

cd vcpkg

echo 
echo Bootstrap ...
echo 

./bootstrap-vcpkg.sh

echo 
echo Installing ...
echo 

./vcpkg install concurrentqueue nlohmann-json boost-fiber[numa] boost-program-options gtest uwebsockets --clean-after-build