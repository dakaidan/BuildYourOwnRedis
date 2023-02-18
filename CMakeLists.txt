cmake_minimum_required(VERSION 3.24)
project(BuildYourOwnRedis)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory(server)
add_subdirectory(client)