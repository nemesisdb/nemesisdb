#!/bin/sh

../vcpkg/installed/x64-linux/tools/flatbuffers/flatc --cpp -o ../core/fbs *.fbs
../vcpkg/installed/x64-linux/tools/flatbuffers/flatc --python -o ../apis/python/ndb/fbs *.fbs