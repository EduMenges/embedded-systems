add_library(hash STATIC hash.c)
target_include_directories(hash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(benchmark benchmark.c)
target_link_libraries(benchmark PRIVATE hash)