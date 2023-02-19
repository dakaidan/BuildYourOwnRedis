project(client)

set(CMAKE_CXX_STANDARD 23)

include_directories(../shared)

add_definitions(-DSOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(client main.cpp)