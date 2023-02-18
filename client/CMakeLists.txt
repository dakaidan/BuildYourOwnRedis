project(client)

set(CMAKE_CXX_STANDARD 23)

include_directories(../shared)

add_executable(client main.cpp)