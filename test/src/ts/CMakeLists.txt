cmake_minimum_required(VERSION 3.20)
project(nemesisdb VERSION 0.1.0 LANGUAGES CXX)

include_directories("../../")
include_directories("../../../")
include_directories("../../clientapi/src/")
include_directories("../../vcpkg/installed/x64-linux/include")
include_directories("../../unordered_dense/include")
include_directories("../../jsoncons/include")

link_directories("../../vcpkg/installed/x64-linux/lib")

# NOTE there are some #define in TestCommon.h

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../bin/)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDB_UNIT_TEST -march=native") 


add_executable(test_orderedseries OrderedSeries.cpp)
add_executable(test_series Series.cpp)
add_executable(test_filter Filter.cpp)


target_compile_features(test_orderedseries PUBLIC cxx_std_20)
target_compile_features(test_series PUBLIC cxx_std_20)
target_compile_features(test_filter PUBLIC cxx_std_20)


target_link_libraries(test_orderedseries -luSockets -lz -lboost_fiber -lboost_program_options -lboost_context -lgtest)
target_link_libraries(test_series -luSockets -lz -lboost_fiber -lboost_program_options -lboost_context -lgtest)
target_link_libraries(test_filter -luSockets -lz -lboost_fiber -lboost_program_options -lboost_context -lgtest)


set(CMAKE_GENERATOR_PLATFORM x64)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
