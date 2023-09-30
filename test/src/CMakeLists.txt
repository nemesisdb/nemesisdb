cmake_minimum_required(VERSION 3.20)
project(fusion VERSION 0.1.0 LANGUAGES CXX)

include_directories("../../")
include_directories("../../../")
include_directories("../../clientapi/src/")
include_directories("../../vcpkg/installed/x64-linux/include")
include_directories("../../unordered_dense/include")

link_directories("../../vcpkg/installed/x64-linux/lib")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../bin/)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DFC_UNIT_TEST -march=native") 


add_executable(test_set Set.cpp)
add_executable(test_setq SetQ.cpp)
add_executable(test_add Add.cpp)
add_executable(test_addq AddQ.cpp)
add_executable(test_get Get.cpp)
add_executable(test_append Append.cpp)
add_executable(test_remove Remove.cpp)
#add_executable(test_rename Rename.cpp)


target_compile_features(test_set PUBLIC cxx_std_20)
target_compile_features(test_setq PUBLIC cxx_std_20)
target_compile_features(test_add PUBLIC cxx_std_20)
target_compile_features(test_addq PUBLIC cxx_std_20)
target_compile_features(test_get PUBLIC cxx_std_20)
target_compile_features(test_append PUBLIC cxx_std_20)
target_compile_features(test_remove PUBLIC cxx_std_20)
#target_compile_features(test_rename PUBLIC cxx_std_20)


target_link_libraries(test_set -luSockets -lz -lboost_fiber -lboost_program_options -lboost_context -lgtest)
target_link_libraries(test_setq -luSockets -lz -lboost_fiber -lboost_program_options -lboost_context -lgtest)
target_link_libraries(test_add -luSockets -lz -lboost_fiber -lboost_program_options -lboost_context -lgtest)
target_link_libraries(test_addq -luSockets -lz -lboost_fiber -lboost_program_options -lboost_context -lgtest)
target_link_libraries(test_get -luSockets -lz -lboost_fiber -lboost_program_options -lboost_context -lgtest)
target_link_libraries(test_append -luSockets -lz -lboost_fiber -lboost_program_options -lboost_context -lgtest)
target_link_libraries(test_remove -luSockets -lz -lboost_fiber -lboost_program_options -lboost_context -lgtest)
#target_link_libraries(test_rename -luSockets -lz -lboost_fiber -lboost_program_options -lboost_context -lgtest)


set(CMAKE_GENERATOR_PLATFORM x64)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
