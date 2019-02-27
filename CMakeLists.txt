cmake_minimum_required(VERSION 3.6)
project(reclass_cli)
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(reclass_cli ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(reclass_cli
ncurses
pthread
stdc++fs
)
