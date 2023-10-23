cmake_minimum_required(VERSION 3.20)


include_directories("../")
include_directories("../vcpkg/installed/x64-linux/include")
include_directories("../unordered_dense/include")
link_directories("../vcpkg/installed/x64-linux/lib")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native") 
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_GENERATOR_PLATFORM x64)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})


add_executable(kvclient kvclient.cpp)


target_compile_features(kvclient PUBLIC cxx_std_20)

target_link_libraries(kvclient PRIVATE -lboost_program_options)

