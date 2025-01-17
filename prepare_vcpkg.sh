#!/bin/sh

cd vcpkg

echo 
echo Bootstrapping ...
echo 

./bootstrap-vcpkg.sh

echo 
echo Installing ...
echo 

# notes:
#   boost-beast used in tests because uwebsockets does not have a client API
#   nlohmann-json used in tests
./vcpkg install nlohmann-json boost-program-options boost-beast gtest uwebsockets plog fixed-string --clean-after-build

cd ..
