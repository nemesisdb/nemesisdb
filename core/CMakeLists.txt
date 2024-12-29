cmake_minimum_required(VERSION 3.20)

include_directories("proto" # for the proto classes 
                    "../"
                    "../vcpkg/installed/x64-linux/include"
                    "../ext/unordered_dense/include")

set(SRC 
    "proto/set.pb.cc"
    "proto/set.pb.cc")

ADD_LIBRARY(core STATIC ${SRC})  

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

target_compile_features(core PUBLIC cxx_std_20)

set_target_properties(core PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

target_link_libraries(core PRIVATE "" -lprotobuf)