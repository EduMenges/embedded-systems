add_library(hash hash.cpp)
target_include_directories(hash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark STATIC hash)