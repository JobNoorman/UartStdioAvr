cmake_minimum_required(VERSION 3.1)
project(UartStdioAvr)

add_compile_options(-Wall)

set(SRC
    src/UartStdio.cpp)

add_library(uart-stdio ${SRC})

target_include_directories(uart-stdio
                           PUBLIC  include/
                           PRIVATE include/UartStdio/)

