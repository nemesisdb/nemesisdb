cmake_minimum_required(VERSION 3.20)
project(fusion VERSION 0.1.0 LANGUAGES CXX)

include_directories("../")
include_directories("../vcpkg/installed/x64-linux/include")
link_directories("../vcpkg/installed/x64-linux/lib")

# include_directories("./")
# include_directories("../uuid_v4")
# include_directories("../unordered_dense/include")


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/bin)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native") # -march is for uuid_v4 allowing native CPU SIMD support
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFCC_KV_METRICS") # to append "_metrics" to each response
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
# set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O1") # for valgind

# NOTE: removed -DBOOST_FIBERS_NO_ATOMICS from cmake_cxx_flags because it causes a compilation error
# https://github.com/boostorg/fiber/issues/301


# set(Boost_USE_STATIC_LIBS ON)
# set(Boost_USE_STATIC_RUNTIME ON)

set(CMAKE_GENERATOR_PLATFORM x64)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

add_executable(fusionserver server.cpp)

target_compile_features(fusionserver PUBLIC cxx_std_20)
target_link_libraries(fusionserver PRIVATE "-Wl,-z,stack-size=2097152" -luSockets -lz -lboost_fiber -lboost_context -lboost_program_options)
#target_link_libraries(fusionserver PRIVATE "-Wl,-z,stack-size=2097152" -lboost_fiber -lboost_context -lboost_program_options)

#include(CPack)
