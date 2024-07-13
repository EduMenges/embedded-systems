cmake_minimum_required(VERSION 3.10)

project(embedded_systems LANGUAGES C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(hash)

if(NOT "${CMAKE_EXE_LINKER_FLAGS}" MATCHES "static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static" CACHE STRING "Linker flags" FORCE)
endif()