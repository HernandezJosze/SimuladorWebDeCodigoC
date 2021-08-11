cmake_minimum_required(VERSION 3.20)
project(SimuladorWebDeCodigoC)

set(CMAKE_CXX_STANDARD 20)

include_directories(lib)

add_executable(SimuladorWebDeCodigoC
        lib/lexer.h
        interprete.cpp)
