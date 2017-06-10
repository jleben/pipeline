cmake_minimum_required(VERSION 3.0)

project(pipeline)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local")

add_executable(pipeline
    pipeline.cpp
    input/tasks.cpp
    input/persistence.cpp
    engine/engine.cpp
    util/filesystem.cpp
)

install(TARGETS pipeline DESTINATION bin)
install(CODE "execute_process(COMMAND pip3 install --user \"${CMAKE_SOURCE_DIR}/python\")")
