cmake_minimum_required(VERSION 3.20)
project(nemesisdb VERSION 0.1.0 LANGUAGES CXX)

include_directories("../")
include_directories("../vcpkg/installed/x64-linux/include")
include_directories("../unordered_dense/include")
include_directories("../jsoncons/include")
link_directories("../vcpkg/installed/x64-linux/lib")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native") 
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O1") # for valgrind


#set(Boost_USE_STATIC_LIBS ON)
#set(Boost_USE_STATIC_RUNTIME ON)

set(CMAKE_GENERATOR_PLATFORM x64)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

add_executable(nemesisdb server.cpp)

#when we have GCC-14
#target_compile_options(nemesisdb PUBLIC -Wnrvo) 
target_compile_features(nemesisdb PUBLIC cxx_std_20)
target_compile_options(nemesisdb PRIVATE -Wall) # TODO reinstate when finished pmr
target_link_libraries(nemesisdb PRIVATE "" -luSockets -lflatbuffers -lz -lboost_program_options)
