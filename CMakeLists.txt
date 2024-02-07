cmake_minimum_required(VERSION 3.20)
project(fusion VERSION 0.1.0 LANGUAGES CXX)

add_subdirectory(server)
add_subdirectory(test/src)
add_subdirectory(clients)
