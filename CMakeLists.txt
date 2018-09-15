cmake_minimum_required(VERSION 3.8)

project(pipeline)

set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local")

add_executable(pipeline
    pipeline.cpp
    input/tasks.cpp
    input/persistence.cpp
    engine/engine.cpp
    util/filesystem.cpp
)

target_compile_features(pipeline PUBLIC cxx_std_14)

install(TARGETS pipeline DESTINATION bin)
install(CODE "execute_process(COMMAND pip3 install --user \"${CMAKE_SOURCE_DIR}/python\")")
