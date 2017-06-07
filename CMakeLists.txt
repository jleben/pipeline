cmake_minimum_required(VERSION 3.0)

project(pipeline)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(pipeline
    pipeline.cpp
    input/simple_work_provider.cpp
    engine/engine.cpp
)
