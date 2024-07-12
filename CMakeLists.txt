cmake_minimum_required(VERSION 3.10)

project(embedded_systems LANGUAGES C)

add_subdirectory(hash)

if(NOT "${CMAKE_EXE_LINKER_FLAGS}" MATCHES "static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static" CACHE STRING "Linker flags" FORCE)
endif()