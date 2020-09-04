cmake_minimum_required(VERSION 3.16)
project(ProgettoAPI C)

set(CMAKE_C_STANDARD 99)

add_executable(ProgettoAPI main.c)
#[[
set(GCC_COVERAGE_COMPILE_FLAGS "-DEVAL -std=gnu11 -O2 -pipe -static -s -lm")
]]
set(GCC_COVERAGE_COMPILE_FLAGS "-DEVAL -std=gnu11 -O2 -pipe -lm -g")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")